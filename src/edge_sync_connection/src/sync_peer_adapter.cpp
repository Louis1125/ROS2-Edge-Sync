#include "edge_sync_connection/sync_peer_adapter.hpp"

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
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
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

void to_proto(const edge_sync::core::Manifest& in,
              edge_sync::protocol::v1::ArtifactManifest* out) {
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
                                 edge_sync::core::IArtifactRepository& local_repository)
    : connections_(connections), local_repository_(local_repository) {
    connections_.set_application_message_handler(
        [this](const Peer& peer, const edge_sync::protocol::MessageEnvelope& env) {
            on_application_message(peer, env);
        });
}

edge_sync::base::Result<edge_sync::core::Manifest> SyncPeerAdapter::request_manifest(
    const edge_sync::base::RobotId& robot_id,
    const edge_sync::base::ArtifactId& artifact_id) {
    edge_sync::protocol::MessageEnvelope env;
    env.set_protocol_version(edge_sync::protocol::kProtocolVersion);
    env.mutable_manifest_request()->set_artifact_id(artifact_id);

    auto response =
        connections_.request(robot_id, std::move(env), std::chrono::milliseconds(5000));
    if (!response) return response.error();

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

void SyncPeerAdapter::on_application_message(
    const Peer& peer, const edge_sync::protocol::MessageEnvelope& env) {
    if (!env.has_manifest_request()) return;

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
}

}  // namespace edge_sync::connection
