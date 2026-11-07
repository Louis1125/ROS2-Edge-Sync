#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "edge_sync_base/logger.hpp"
#include "edge_sync_base/result.hpp"
#include "edge_sync_connection/peer.hpp"
#include "edge_sync_connection/retry_policy.hpp"
#include "edge_sync_connection/session.hpp"
#include "edge_sync_connection/timeout_manager.hpp"
#include "edge_sync_connection/transport_interface.hpp"
#include "edge_sync_protocol/message_codec.hpp"

namespace edge_sync::connection {

using ApplicationMessageHandler =
    std::function<void(const Peer&, const edge_sync::protocol::MessageEnvelope&)>;

// Owns the transport, the peer registry, handshaking, session lifecycle and
// request/response correlation. This is the single entry point for all
// cross-robot communication. It depends only on the ITransport interface.
//
// Thread-safety: transport callbacks run on the transport's single I/O thread;
// start/connect_peer/request/stop may be called from the owner thread. The
// pending-request map is guarded by its own mutex.
class ConnectionManager {
   public:
    ConnectionManager(std::unique_ptr<ITransport> transport, edge_sync::base::Logger& logger,
                      RetryPolicy retry = {}, TimeoutConfig timeouts = {});

    void set_local_identity(std::string robot_id, std::string public_key_id = {});

    // Enable handshake authentication: outbound connections present
    // HMAC-SHA256(secret, robot_id); inbound connections are rejected if it does
    // not verify. An empty secret disables authentication.
    void set_shared_secret(std::string secret);

    edge_sync::base::Result<void> start(const Endpoint& listen_endpoint);
    edge_sync::base::Result<void> connect_peer(const Endpoint& endpoint);

    // Send `envelope` to `robot_id` and block up to `timeout` for a response
    // envelope carrying the same request_id. This is the request/response
    // primitive the sync engine builds on.
    edge_sync::base::Result<edge_sync::protocol::MessageEnvelope> request(
        const std::string& robot_id, edge_sync::protocol::MessageEnvelope envelope,
        std::chrono::milliseconds timeout);

    edge_sync::base::Result<Session::Id> open_session(const std::string& robot_id);
    edge_sync::base::Result<void> close_session(Session::Id id);

    void send(const std::string& robot_id, const edge_sync::protocol::MessageEnvelope& envelope);
    void set_application_message_handler(ApplicationMessageHandler handler);

    // Notified when a peer finishes the handshake (connected) or drops.
    void set_peer_event_handler(std::function<void(const std::string&)> on_connected,
                                std::function<void(const std::string&)> on_disconnected);
    void stop();

    [[nodiscard]] bool is_connected(const std::string& robot_id) const;

   private:
    void on_accepted(ConnectionId id, const Endpoint& endpoint);
    void on_message(ConnectionId id, ByteBuffer data);
    void on_disconnected(ConnectionId id);

    void handle_handshake_request(ConnectionId id, const Endpoint& endpoint,
                                  const edge_sync::protocol::MessageEnvelope& env);
    void handle_handshake_response(ConnectionId id, const Endpoint& endpoint,
                                   const edge_sync::protocol::MessageEnvelope& env);
    void send_envelope(ConnectionId id, const edge_sync::protocol::MessageEnvelope& env);
    void schedule_reconnect(const Endpoint& endpoint);
    void erase_connection(ConnectionId id);

    std::optional<ConnectionId> connection_for(const std::string& robot_id) const;

    std::unique_ptr<ITransport> transport_;
    edge_sync::base::Logger& logger_;

    std::string local_robot_id_;
    std::string local_public_key_id_;
    std::string shared_secret_;

    std::map<ConnectionId, Peer> peers_;          // conn -> peer (after handshake)
    std::map<ConnectionId, Endpoint> endpoints_;  // conn -> remote endpoint
    std::map<ConnectionId, bool> outbound_;       // conn -> we initiated it
    std::map<std::string, ConnectionId> conn_by_robot_;

    std::map<Session::Id, std::string> sessions_;  // session -> robot id
    Session::Id next_session_id_{1};

    std::uint64_t next_request_id_{1};
    std::mutex pending_mu_;
    std::map<std::string, std::shared_ptr<std::promise<
                              edge_sync::base::Result<edge_sync::protocol::MessageEnvelope>>>>
        pending_;

    RetryPolicy retry_;
    TimeoutConfig timeouts_;
    ApplicationMessageHandler app_handler_;
    std::function<void(const std::string&)> peer_connected_;
    std::function<void(const std::string&)> peer_disconnected_;

    std::atomic<bool> stopping_{false};
    std::vector<std::jthread> reconnect_threads_;
};

}  // namespace edge_sync::connection
