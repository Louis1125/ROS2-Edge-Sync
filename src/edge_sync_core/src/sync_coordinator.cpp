#include "edge_sync_core/sync_coordinator.hpp"

namespace edge_sync::core {

SyncCoordinator::SyncCoordinator(IRemotePeer& peers, IArtifactRepository& local_repository)
    : peers_(peers), local_repository_(local_repository) {}

edge_sync::base::Result<SyncPlan> SyncCoordinator::prepare(
    const edge_sync::base::RobotId& source_robot,
    const edge_sync::base::ArtifactId& artifact_id) {
    auto remote = peers_.request_manifest(source_robot, artifact_id);
    if (!remote) return remote.error();

    SyncPlan plan;
    plan.artifact_id = artifact_id;
    plan.remote_version = remote.value().version;

    const auto local_exists = local_repository_.exists(artifact_id);
    const bool needs_all = !local_exists.ok() || !local_exists.value();
    if (needs_all) {
        plan.needed_files = remote.value().files;
    }
    return plan;
}

}  // namespace edge_sync::core
