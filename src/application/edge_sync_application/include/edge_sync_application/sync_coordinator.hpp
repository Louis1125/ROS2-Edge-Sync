#pragma once

#include <string>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_connection/connection_manager.hpp"
#include "edge_sync_domain/repository_interfaces.hpp"

namespace edge_sync::application {

class SyncCoordinator {
public:
    SyncCoordinator(edge_sync::connection::ConnectionManager& connections,
                    edge_sync::domain::IArtifactRepository& repository);

    edge_sync::base::Result<void> start_sync(const std::string& target_robot_id,
                                             const edge_sync::base::ArtifactId& artifact_id);

private:
    edge_sync::connection::ConnectionManager& connections_;
    edge_sync::domain::IArtifactRepository& repository_;
};

}  // namespace edge_sync::application
