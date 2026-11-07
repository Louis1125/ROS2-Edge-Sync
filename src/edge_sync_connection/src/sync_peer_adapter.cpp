#include "edge_sync_connection/sync_peer_adapter.hpp"

#include <algorithm>
#include <set>
#include <string>

namespace edge_sync::connection {
namespace {

std::string bytes_to_hex(const std::string& bytes) {
    static const char* d = "0123456789abcdef";
    std::string out(bytes.size() * 2, '0');
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        const unsigned char c = static_cast<unsigned char>(bytes[i]);
        out[i * 2] = d[c >> 4];
        out[i * 2 + 1] = d[c & 0xf];
    }
    return out;
}

std::string hex_to_bytes(const std::string& hex) {
    auto val = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    };
    std::string out(hex.size() / 2, '\0');
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        out[i / 2] = static_cast<char>((val(hex[i]) << 4) | val(hex[i + 1]));
    }
    return out;
}

edge_sync::core::Manifest to_core(const edge_sync::protocol::v1::ArtifactManifest& m) {
    edge_sync::core::Manifest out;
    out.artifact_id = m.artifact_id();
    out.version = {m.version_major(), m.version_minor(), m.version_patch()};
    out.schema_version = m.schema_version();
    out.total_size = m.total_size();
    out.chunk_size = m.chunk_size();
    out.artifact_hash = bytes_to_hex(m.artifact_hash().digest());
    for (const auto& c : m.chunks()) {
        edge_sync::core::ChunkDescriptor d;
        d.index = c.index();
        d.offset = c.offset();
        d.size = c.size();
        d.hash = bytes_to_hex(c.hash().digest());
        out.chunks.push_back(std::move(d));
    }
    return out;
}

void to_proto(const edge_sync::core::Manifest& in, edge_sync::protocol::v1::ArtifactManifest* out) {
    out->set_artifact_id(in.artifact_id);
    out->set_version_major(in.version.major);
    out->set_version_minor(in.version.minor);
    out->set_version_patch(in.version.patch);
    out->set_schema_version(in.schema_version);
    out->set_total_size(in.total_size);
    out->set_chunk_size(in.chunk_size);
    out->mutable_artifact_hash()->set_algorithm("sha256");
    out->mutable_artifact_hash()->set_digest(hex_to_bytes(in.artifact_hash));
    for (const auto& c : in.chunks) {
        auto* pc = out->add_chunks();
        pc->set_index(c.index);
        pc->set_offset(c.offset);
        pc->set_size(c.size);
        pc->mutable_hash()->set_algorithm("sha256");
        pc->mutable_hash()->set_digest(hex_to_bytes(c.hash));
    }
}

}  // namespace

SyncPeerAdapter::SyncPeerAdapter(ConnectionManager& connections,
                                 edge_sync::core::IArtifactRepository& local_repository,
                                 edge_sync::core::IChunkProvider& chunks)
    : connections_(connections), local_repository_(local_repository), chunks_(chunks) {
    connections_.set_application_message_handler(
        [this](const Peer& peer, const edge_sync::protocol::MessageEnvelope& env) {
            on_application_message(peer, env);
        });
    worker_ = std::jthread([this](std::stop_token stop) { worker_loop(stop); });
}

SyncPeerAdapter::~SyncPeerAdapter() {
    stop();
}

void SyncPeerAdapter::stop() {
    if (stopped_.exchange(true))
        return;
    if (worker_.joinable()) {
        worker_.request_stop();
        queue_cv_.notify_all();
        worker_.join();
    }
}

edge_sync::base::Result<edge_sync::core::Manifest> SyncPeerAdapter::request_manifest(
    const edge_sync::base::RobotId& robot_id, const edge_sync::base::ArtifactId& artifact_id) {
    edge_sync::protocol::MessageEnvelope env;
    env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    env.mutable_manifest_request()->set_artifact_id(artifact_id);

    auto response = connections_.request(robot_id, std::move(env), std::chrono::milliseconds(5000));
    if (!response)
        return response.error();

    if (!response.value().has_manifest_response()) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ProtocolMismatch,
                                      "expected manifest response"};
    }
    const auto& mr = response.value().manifest_response();
    if (!mr.found()) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "artifact not found on '" + robot_id + "'"};
    }
    return to_core(mr.manifest());
}

edge_sync::base::Result<void> SyncPeerAdapter::pull_chunks(
    const edge_sync::base::RobotId& source, const edge_sync::base::ArtifactId& artifact_id,
    const std::vector<std::uint64_t>& chunk_indices, edge_sync::core::IChunkReceiver& receiver) {
    if (chunk_indices.empty())
        return {};

    const std::string transfer_id = source + "-" + std::to_string(transfer_seq_++);
    auto ctx = std::make_shared<PullContext>();
    {
        std::lock_guard lk(pulls_mu_);
        pulls_[transfer_id] = ctx;
    }
    auto unregister = [&] {
        std::lock_guard lk(pulls_mu_);
        pulls_.erase(transfer_id);
    };

    // Open the transfer session (control plane of the transfer).
    {
        edge_sync::protocol::MessageEnvelope env;
        env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
        auto* req = env.mutable_transfer_request();
        req->set_transfer_id(transfer_id);
        req->set_artifact_id(artifact_id);
        for (const auto idx : chunk_indices) req->add_chunk_indices(idx);

        auto resp = connections_.request(source, std::move(env), std::chrono::milliseconds(5000));
        if (!resp) {
            unregister();
            return resp.error();
        }
        if (!resp.value().has_transfer_response() || !resp.value().transfer_response().accepted()) {
            unregister();
            return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                          resp.value().has_transfer_response()
                                              ? resp.value().transfer_response().reason()
                                              : "transfer rejected"};
        }
    }

    constexpr std::size_t kWindow = 4;
    std::set<std::uint64_t> outstanding(chunk_indices.begin(), chunk_indices.end());
    std::vector<std::uint64_t> inflight;
    std::size_t next = 0;

    auto request_next = [&] {
        while (inflight.size() < kWindow && next < chunk_indices.size()) {
            const auto idx = chunk_indices[next++];
            send_chunk_request(source, transfer_id, artifact_id, idx);
            inflight.push_back(idx);
        }
    };
    request_next();

    int timeouts = 0;
    constexpr int kMaxTimeouts = 5;
    while (!outstanding.empty()) {
        std::unique_lock lk(ctx->mu);
        if (ctx->failed)
            break;
        if (ctx->data.empty()) {
            const bool got = ctx->cv.wait_for(lk, std::chrono::seconds(3),
                                              [&] { return !ctx->data.empty() || ctx->failed; });
            if (!got) {
                // Re-request every outstanding chunk (receiver-authoritative resume).
                if (++timeouts > kMaxTimeouts) {
                    unregister();
                    return edge_sync::base::Error{edge_sync::base::ErrorCode::Timeout,
                                                  "transfer timed out"};
                }
                lk.unlock();
                for (const auto idx : outstanding) {
                    send_chunk_request(source, transfer_id, artifact_id, idx);
                }
                continue;
            }
        }

        auto it = ctx->data.begin();
        const std::uint64_t idx = it->first;
        ByteBuffer data = std::move(it->second);
        ctx->data.erase(it);
        lk.unlock();

        auto r = receiver.on_chunk(idx, std::move(data));
        if (!r.ok()) {
            std::lock_guard lk2(ctx->mu);
            ctx->failed = true;
            ctx->error = r.error().message;
            ctx->cv.notify_all();
            break;
        }

        send_chunk_ack(source, transfer_id, idx);

        lk.lock();
        outstanding.erase(idx);
        inflight.erase(std::remove(inflight.begin(), inflight.end(), idx), inflight.end());
        lk.unlock();

        request_next();
    }

    unregister();
    if (ctx->failed) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::Internal, ctx->error};
    }
    return {};
}

void SyncPeerAdapter::on_application_message(const Peer& peer,
                                             const edge_sync::protocol::MessageEnvelope& env) {
    if (env.has_manifest_request()) {
        const auto& req = env.manifest_request();
        const auto local = local_repository_.load(req.artifact_id());
        edge_sync::protocol::MessageEnvelope response;
        response.set_protocol_version(edge_sync::protocol::kProtocolVersion);
        response.set_request_id(env.request_id());
        auto* mr = response.mutable_manifest_response();
        if (local.ok()) {
            mr->set_found(true);
            to_proto(local.value(), mr->mutable_manifest());
        } else {
            mr->set_found(false);
        }
        connections_.send(peer.identity.robot_id, response);
        return;
    }

    if (env.has_transfer_request()) {
        const auto& req = env.transfer_request();
        const auto exists = local_repository_.exists(req.artifact_id());
        const bool have = exists.ok() && exists.value();
        edge_sync::protocol::MessageEnvelope response;
        response.set_protocol_version(edge_sync::protocol::kProtocolVersion);
        response.set_request_id(env.request_id());
        auto* tr = response.mutable_transfer_response();
        tr->set_transfer_id(req.transfer_id());
        tr->set_accepted(have);
        if (!have)
            tr->set_reason("artifact not found");
        connections_.send(peer.identity.robot_id, response);
        return;
    }

    if (env.has_chunk_request()) {
        const auto& req = env.chunk_request();
        ServeTask task{peer.identity.robot_id, req.transfer_id(), req.artifact_id(),
                       req.chunk_index()};
        {
            std::lock_guard lk(queue_mu_);
            queue_.push(std::move(task));
        }
        queue_cv_.notify_all();
        return;
    }

    if (env.has_chunk_data()) {
        const auto& cd = env.chunk_data();
        std::lock_guard lk(pulls_mu_);
        const auto it = pulls_.find(cd.transfer_id());
        if (it == pulls_.end())
            return;  // late data for a finished transfer
        auto& ctx = it->second;
        std::lock_guard clk(ctx->mu);
        ctx->data[cd.chunk_index()] = ByteBuffer(cd.data().begin(), cd.data().end());
        ctx->cv.notify_all();
        return;
    }

    // chunk_ack is informational; the receiver is authoritative for resume.
}

void SyncPeerAdapter::send_chunk_request(const std::string& robot, const std::string& transfer_id,
                                         const std::string& artifact_id,
                                         std::uint64_t chunk_index) {
    edge_sync::protocol::MessageEnvelope env;
    env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    auto* req = env.mutable_chunk_request();
    req->set_transfer_id(transfer_id);
    req->set_artifact_id(artifact_id);
    req->set_chunk_index(chunk_index);
    connections_.send(robot, env);
}

void SyncPeerAdapter::send_chunk_ack(const std::string& robot, const std::string& transfer_id,
                                     std::uint64_t chunk_index) {
    edge_sync::protocol::MessageEnvelope env;
    env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    auto* ack = env.mutable_chunk_ack();
    ack->set_transfer_id(transfer_id);
    ack->set_chunk_index(chunk_index);
    connections_.send(robot, env);
}

void SyncPeerAdapter::worker_loop(std::stop_token stop) {
    while (!stop.stop_requested()) {
        ServeTask task;
        {
            std::unique_lock lk(queue_mu_);
            queue_cv_.wait(lk, stop, [&] { return !queue_.empty(); });
            if (stop.stop_requested())
                return;
            task = std::move(queue_.front());
            queue_.pop();
        }

        auto manifest = local_repository_.load(task.artifact_id);
        if (!manifest.ok())
            continue;

        edge_sync::base::Hash hash;
        bool found = false;
        for (const auto& c : manifest.value().chunks) {
            if (c.index == task.chunk_index) {
                hash = c.hash;
                found = true;
                break;
            }
        }
        if (!found)
            continue;

        auto data = chunks_.get_chunk(hash);
        if (!data.ok())
            continue;

        edge_sync::protocol::MessageEnvelope env;
        env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
        auto* cd = env.mutable_chunk_data();
        cd->set_transfer_id(task.transfer_id);
        cd->set_chunk_index(task.chunk_index);
        cd->set_data(std::string(data.value().begin(), data.value().end()));
        connections_.send(task.target_robot, env);
    }
}

}  // namespace edge_sync::connection
