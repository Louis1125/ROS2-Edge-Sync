#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/remote_peer.hpp"
#include "edge_sync_core/repository_interfaces.hpp"
#include "edge_sync_core/sync_plan.hpp"

namespace edge_sync::core {

// Application service that orchestrates a synchronization session. It depends
// only on the IRemotePeer and IArtifactRepository ports; the runtime injects the
// concrete implementations (connection adapter + storage). This is the single
// entry point that the ROS 2 adapter and the HTTP backend will both call.
class SyncCoordinator {
public:
    SyncCoordinator(IRemotePeer& peers, IArtifactRepository& local_repository);

    // Fetch the remote manifest and compute which files must be fetched.
    // (M1-A: all remote files when the artifact is absent locally; M1-B adds a
    // real content diff.)
    edge_sync::base::Result<SyncPlan> prepare(const edge_sync::base::RobotId& source_robot,
                                              const edge_sync::base::ArtifactId& artifact_id);

private:
    IRemotePeer& peers_;
    IArtifactRepository& local_repository_;
};

}  // namespace edge_sync::core
