#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::core {

// Port the sync engine uses to reach remote robots. Implemented by the
// connection layer over the ConnectionManager. Core depends only on this
// interface, never on a concrete transport.
class IRemotePeer {
public:
    virtual ~IRemotePeer() = default;

    virtual edge_sync::base::Result<Manifest> request_manifest(
        const edge_sync::base::RobotId& robot_id,
        const edge_sync::base::ArtifactId& artifact_id) = 0;
};

}  // namespace edge_sync::core
