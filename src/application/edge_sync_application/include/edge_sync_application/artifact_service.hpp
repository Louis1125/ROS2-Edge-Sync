#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::application {

class ArtifactService {
public:
    edge_sync::base::Result<void> stage(const edge_sync::base::ArtifactId& artifact_id);
    edge_sync::base::Result<void> activate(const edge_sync::base::ArtifactId& artifact_id);
};

}  // namespace edge_sync::application
