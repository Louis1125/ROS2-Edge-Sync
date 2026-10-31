#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_domain/manifest.hpp"

namespace edge_sync::application {

class ManifestService {
public:
    edge_sync::base::Result<edge_sync::domain::Manifest> build(
        const edge_sync::base::ArtifactId& artifact_id);
};

}  // namespace edge_sync::application
