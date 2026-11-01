#pragma once

#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_core/remote_peer.hpp"
#include "edge_sync_core/repository_interfaces.hpp"

namespace edge_sync::connection {

// Adapts the ConnectionManager to the core::IRemotePeer port. This is where the
// wire protocol (protobuf) is translated to/from the core domain types, so the
// sync engine never sees transport or serialization details.
class SyncPeerAdapter final : public edge_sync::core::IRemotePeer {
public:
    SyncPeerAdapter(ConnectionManager& connections,
                    edge_sync::core::IArtifactRepository& local_repository);

    edge_sync::base::Result<edge_sync::core::Manifest> request_manifest(
        const edge_sync::base::RobotId& robot_id,
        const edge_sync::base::ArtifactId& artifact_id) override;

    // Incoming application-message handler (registered on the ConnectionManager
    // at construction): answers manifest requests from the local repository.
    void on_application_message(const Peer& peer,
                                const edge_sync::protocol::MessageEnvelope& env);

private:
    ConnectionManager& connections_;
    edge_sync::core::IArtifactRepository& local_repository_;
};

}  // namespace edge_sync::connection
