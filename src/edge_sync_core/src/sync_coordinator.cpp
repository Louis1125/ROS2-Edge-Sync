#include "edge_sync_core/sync_coordinator.hpp"

#include <optional>

#include "edge_sync_core/sync_planner.hpp"

namespace edge_sync::core {

SyncCoordinator::SyncCoordinator(IRemotePeer& peers, IArtifactRepository& local_repository)
    : peers_(peers), local_repository_(local_repository) {}

edge_sync::base::Result<SyncPlan> SyncCoordinator::prepare(
    const edge_sync::base::RobotId& source_robot, const edge_sync::base::ArtifactId& artifact_id) {
    auto remote = peers_.request_manifest(source_robot, artifact_id);
    if (!remote)
        return remote.error();

    std::optional<Manifest> local;
    const auto exists = local_repository_.exists(artifact_id);
    if (exists.ok() && exists.value()) {
        const auto loaded = local_repository_.load(artifact_id);
        if (loaded.ok())
            local = loaded.value();
    }

    SyncPlanner planner;
    return planner.plan(local, remote.value());
}

}  // namespace edge_sync::core
