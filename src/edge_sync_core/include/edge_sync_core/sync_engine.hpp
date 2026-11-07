#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/artifact_store.hpp"
#include "edge_sync_core/remote_peer.hpp"
#include "edge_sync_core/repository_interfaces.hpp"
#include "edge_sync_core/sync_plan.hpp"
#include "edge_sync_core/transfer.hpp"

namespace edge_sync::core {

// The full synchronization use case: plan the diff, stream missing chunks with
// verification, then atomically activate. Both the ROS 2 action and the HTTP
// backend invoke this same service.
class SyncEngine {
   public:
    SyncEngine(IRemotePeer& peers, IArtifactRepository& repository, ITransfer& transfer,
               IArtifactStore& store);

    edge_sync::base::Result<SyncOutcome> synchronize(
        const edge_sync::base::RobotId& source, const edge_sync::base::ArtifactId& artifact_id);

   private:
    IRemotePeer& peers_;
    IArtifactRepository& repository_;
    ITransfer& transfer_;
    IArtifactStore& store_;
};

}  // namespace edge_sync::core
