#include "edge_sync_connection/connection_manager.hpp"

#include <condition_variable>
#include <mutex>
#include <utility>

#include "edge_sync_base/hmac.hpp"

namespace edge_sync::connection {
namespace {

void sleep_for(std::chrono::milliseconds duration, const std::stop_token& stop) {
    std::mutex m;
    std::condition_variable_any cv;
    std::unique_lock lk(m);
    cv.wait_for(lk, stop, duration, [] { return false; });
}

constexpr std::size_t kMaxPendingRequests = 1024;

std::string endpoint_string(const Endpoint& e) {
    return e.address + ":" + std::to_string(e.port);
}

}  // namespace

ConnectionManager::ConnectionManager(std::unique_ptr<ITransport> transport,
                                     edge_sync::base::Logger& logger, RetryPolicy retry,
                                     TimeoutConfig timeouts)
    : transport_(std::move(transport)), logger_(logger), retry_(retry), timeouts_(timeouts) {
    transport_->set_callbacks(TransportCallbacks{
        [this](ConnectionId id, const Endpoint& ep) { on_accepted(id, ep); },
        [this](ConnectionId id, ByteBuffer data) { on_message(id, std::move(data)); },
        [this](ConnectionId id) { on_disconnected(id); },
    });
}

void ConnectionManager::set_local_identity(std::string robot_id, std::string public_key_id) {
    local_robot_id_ = std::move(robot_id);
    local_public_key_id_ = std::move(public_key_id);
}

void ConnectionManager::set_shared_secret(std::string secret) {
    shared_secret_ = std::move(secret);
}

edge_sync::base::Result<void> ConnectionManager::start(const Endpoint& listen_endpoint) {
    return transport_->start(listen_endpoint);
}

edge_sync::base::Result<void> ConnectionManager::connect_peer(const Endpoint& endpoint) {
    auto result = transport_->connect(endpoint);
    if (!result)
        return result.error();

    const ConnectionId id = result.value();
    endpoints_[id] = endpoint;
    outbound_[id] = true;
    peers_[id] = Peer{RobotIdentity{}, endpoint, ConnectionState::Connecting};

    edge_sync::protocol::MessageEnvelope env;
    env.set_robot_id(local_robot_id_);
    env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    auto* req = env.mutable_handshake_request();
    req->set_robot_id(local_robot_id_);
    req->set_public_key_id(local_public_key_id_);
    req->set_protocol_version(edge_sync::protocol::kProtocolVersion);
    if (!shared_secret_.empty()) {
        req->set_auth_token(edge_sync::base::hmac_sha256(shared_secret_, local_robot_id_));
    }
    send_envelope(id, env);

    logger_.log(edge_sync::base::LogLevel::Info, "connecting to " + endpoint_string(endpoint));
    return {};
}

edge_sync::base::Result<edge_sync::protocol::MessageEnvelope> ConnectionManager::request(
    const std::string& robot_id, edge_sync::protocol::MessageEnvelope envelope,
    std::chrono::milliseconds timeout) {
    const auto conn = connection_for(robot_id);
    if (!conn) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                      "not connected to '" + robot_id + "'"};
    }

    const std::string request_id = std::to_string(next_request_id_++);
    envelope.set_request_id(request_id);

    using Pending = std::promise<edge_sync::base::Result<edge_sync::protocol::MessageEnvelope>>;
    auto promise = std::make_shared<Pending>();
    auto future = promise->get_future();
    {
        std::lock_guard lk(pending_mu_);
        if (pending_.size() >= kMaxPendingRequests) {
            return edge_sync::base::Error{edge_sync::base::ErrorCode::Internal,
                                          "too many pending requests"};
        }
        pending_[request_id] = std::move(promise);
    }

    send_envelope(*conn, envelope);

    if (future.wait_for(timeout) != std::future_status::ready) {
        std::lock_guard lk(pending_mu_);
        pending_.erase(request_id);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::Timeout, "request timed out"};
    }
    return future.get();
}

void ConnectionManager::on_accepted(ConnectionId id, const Endpoint& endpoint) {
    endpoints_[id] = endpoint;
    outbound_[id] = false;
    peers_[id] = Peer{RobotIdentity{}, endpoint, ConnectionState::Connecting};
    logger_.log(edge_sync::base::LogLevel::Info,
                "accepted connection from " + endpoint_string(endpoint));
}

void ConnectionManager::on_message(ConnectionId id, ByteBuffer data) {
    auto parsed = edge_sync::protocol::parse(data);
    if (!parsed) {
        logger_.log(edge_sync::base::LogLevel::Warn,
                    "dropping undecodable message: " + parsed.error().message);
        return;
    }
    const auto& env = parsed.value();

    // Resolve a pending request first (responses carry the requester's request_id).
    if (!env.request_id().empty()) {
        std::lock_guard lk(pending_mu_);
        const auto it = pending_.find(env.request_id());
        if (it != pending_.end()) {
            it->second->set_value(
                edge_sync::base::Result<edge_sync::protocol::MessageEnvelope>{env});
            pending_.erase(it);
            return;
        }
    }

    if (env.has_handshake_request()) {
        handle_handshake_request(id, endpoints_[id], env);
        return;
    }
    if (env.has_handshake_response()) {
        handle_handshake_response(id, endpoints_[id], env);
        return;
    }

    const auto it = peers_.find(id);
    if (it == peers_.end() || it->second.state != ConnectionState::Connected) {
        logger_.log(edge_sync::base::LogLevel::Warn,
                    "dropping application message from non-connected peer");
        return;
    }
    if (app_handler_)
        app_handler_(it->second, env);
}

void ConnectionManager::handle_handshake_request(ConnectionId id, const Endpoint&,
                                                 const edge_sync::protocol::MessageEnvelope& env) {
    const auto& req = env.handshake_request();

    edge_sync::protocol::MessageEnvelope resp;
    resp.set_robot_id(local_robot_id_);
    resp.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    auto* hr = resp.mutable_handshake_response();
    hr->set_robot_id(local_robot_id_);
    hr->set_protocol_version(edge_sync::protocol::kProtocolVersion);

    if (req.protocol_version() != edge_sync::protocol::kProtocolVersion) {
        hr->set_accepted(false);
        hr->set_reason("protocol version mismatch");
        send_envelope(id, resp);
        transport_->close(id);
        logger_.log(edge_sync::base::LogLevel::Error,
                    "rejected handshake from " + req.robot_id() + ": version mismatch");
        return;
    }

    if (!shared_secret_.empty()) {
        const auto expected = edge_sync::base::hmac_sha256(shared_secret_, req.robot_id());
        if (req.auth_token() != expected) {
            hr->set_accepted(false);
            hr->set_reason("authentication failed");
            send_envelope(id, resp);
            transport_->close(id);
            logger_.log(edge_sync::base::LogLevel::Error,
                        "rejected handshake from '" + req.robot_id() + "': authentication failed");
            return;
        }
    }

    auto& peer = peers_[id];
    peer.identity = RobotIdentity{req.robot_id(), req.public_key_id()};
    peer.state = ConnectionState::Connected;
    conn_by_robot_[req.robot_id()] = id;

    hr->set_accepted(true);
    send_envelope(id, resp);
    logger_.log(edge_sync::base::LogLevel::Info,
                "handshake complete (inbound) with '" + req.robot_id() + "'");
    if (peer_connected_)
        peer_connected_(req.robot_id());
}

void ConnectionManager::handle_handshake_response(ConnectionId id, const Endpoint&,
                                                  const edge_sync::protocol::MessageEnvelope& env) {
    const auto& resp = env.handshake_response();
    const auto it = peers_.find(id);
    if (it == peers_.end())
        return;

    if (!resp.accepted()) {
        it->second.state = ConnectionState::Failed;
        logger_.log(edge_sync::base::LogLevel::Error, "handshake rejected: " + resp.reason());
        transport_->close(id);
        return;
    }

    it->second.identity = RobotIdentity{resp.robot_id(), ""};
    it->second.state = ConnectionState::Connected;
    conn_by_robot_[resp.robot_id()] = id;
    logger_.log(edge_sync::base::LogLevel::Info,
                "handshake complete (outbound) with '" + resp.robot_id() + "'");
    if (peer_connected_)
        peer_connected_(resp.robot_id());
}

void ConnectionManager::on_disconnected(ConnectionId id) {
    const auto pit = peers_.find(id);
    const std::string robot_id = (pit != peers_.end()) ? pit->second.identity.robot_id : "";
    const bool was_outbound = outbound_.count(id) && outbound_[id];
    const Endpoint endpoint = endpoints_[id];

    if (!robot_id.empty())
        conn_by_robot_.erase(robot_id);
    erase_connection(id);
    if (peer_disconnected_ && !robot_id.empty())
        peer_disconnected_(robot_id);

    logger_.log(edge_sync::base::LogLevel::Warn,
                "connection lost" + (robot_id.empty() ? "" : " with '" + robot_id + "'"));

    if (was_outbound && !stopping_)
        schedule_reconnect(endpoint);
}

void ConnectionManager::schedule_reconnect(const Endpoint& endpoint) {
    reconnect_threads_.emplace_back([this, endpoint](std::stop_token stop) {
        auto delay = retry_.base_delay;
        for (std::uint32_t attempt = 1; attempt <= retry_.max_attempts; ++attempt) {
            sleep_for(delay, stop);
            if (stop.stop_requested() || stopping_)
                return;

            logger_.log(edge_sync::base::LogLevel::Info, "reconnect attempt " +
                                                             std::to_string(attempt) + " to " +
                                                             endpoint_string(endpoint));
            if (connect_peer(endpoint).ok()) {
                logger_.log(edge_sync::base::LogLevel::Info,
                            "reconnected to " + endpoint_string(endpoint));
                return;
            }
            delay = std::chrono::duration_cast<std::chrono::milliseconds>(delay *
                                                                          retry_.backoff_factor);
            if (delay > retry_.max_delay)
                delay = retry_.max_delay;
        }
        logger_.log(edge_sync::base::LogLevel::Error,
                    "gave up reconnecting to " + endpoint_string(endpoint));
    });
}

edge_sync::base::Result<Session::Id> ConnectionManager::open_session(const std::string& robot_id) {
    if (!connection_for(robot_id)) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                      "not connected to '" + robot_id + "'"};
    }
    const Session::Id id = next_session_id_++;
    sessions_[id] = robot_id;
    logger_.log(edge_sync::base::LogLevel::Info,
                "session " + std::to_string(id) + " opened with '" + robot_id + "'");
    return id;
}

edge_sync::base::Result<void> ConnectionManager::close_session(Session::Id id) {
    const auto it = sessions_.find(id);
    if (it == sessions_.end()) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::InvalidArgument,
                                      "unknown session " + std::to_string(id)};
    }
    logger_.log(edge_sync::base::LogLevel::Info,
                "session " + std::to_string(id) + " closed with '" + it->second + "'");
    sessions_.erase(it);
    return {};
}

void ConnectionManager::send(const std::string& robot_id,
                             const edge_sync::protocol::MessageEnvelope& envelope) {
    const auto conn = connection_for(robot_id);
    if (!conn) {
        logger_.log(edge_sync::base::LogLevel::Warn,
                    "cannot send to '" + robot_id + "': not connected");
        return;
    }
    send_envelope(*conn, envelope);
}

void ConnectionManager::send_envelope(ConnectionId id,
                                      const edge_sync::protocol::MessageEnvelope& env) {
    auto bytes = edge_sync::protocol::serialize(env);
    if (!bytes) {
        logger_.log(edge_sync::base::LogLevel::Error,
                    "failed to serialize envelope: " + bytes.error().message);
        return;
    }
    transport_->send(id, bytes.value());
}

void ConnectionManager::set_application_message_handler(ApplicationMessageHandler handler) {
    app_handler_ = std::move(handler);
}

void ConnectionManager::set_peer_event_handler(
    std::function<void(const std::string&)> on_connected,
    std::function<void(const std::string&)> on_disconnected) {
    peer_connected_ = std::move(on_connected);
    peer_disconnected_ = std::move(on_disconnected);
}

bool ConnectionManager::is_connected(const std::string& robot_id) const {
    return connection_for(robot_id).has_value();
}

std::optional<ConnectionId> ConnectionManager::connection_for(const std::string& robot_id) const {
    const auto it = conn_by_robot_.find(robot_id);
    if (it == conn_by_robot_.end())
        return std::nullopt;
    return it->second;
}

void ConnectionManager::erase_connection(ConnectionId id) {
    peers_.erase(id);
    endpoints_.erase(id);
    outbound_.erase(id);
}

void ConnectionManager::stop() {
    if (stopping_.exchange(true))
        return;
    for (auto& t : reconnect_threads_) t.request_stop();
    transport_->stop();
    for (auto& t : reconnect_threads_) {
        if (t.joinable())
            t.join();
    }
    reconnect_threads_.clear();
    {
        std::lock_guard lk(pending_mu_);
        for (auto& [id, p] : pending_) {
            p->set_value(edge_sync::base::Result<edge_sync::protocol::MessageEnvelope>{
                edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                       "connection manager stopped"}});
        }
        pending_.clear();
    }
    conn_by_robot_.clear();
    sessions_.clear();
    peers_.clear();
    endpoints_.clear();
    outbound_.clear();
}

}  // namespace edge_sync::connection
