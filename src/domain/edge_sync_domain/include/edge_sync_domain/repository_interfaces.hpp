#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_domain/manifest.hpp"

namespace edge_sync::domain {

class IArtifactRepository {
public:
    virtual ~IArtifactRepository() = default;

    virtual edge_sync::base::Result<void> store(const Manifest& manifest) = 0;
    virtual edge_sync::base::Result<Manifest> load(
        const edge_sync::base::ArtifactId& artifact_id) = 0;
    virtual edge_sync::base::Result<bool> exists(
        const edge_sync::base::ArtifactId& artifact_id) = 0;
};

}  // namespace edge_sync::domain
