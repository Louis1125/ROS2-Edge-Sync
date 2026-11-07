#pragma once

#include <optional>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::core {

// Port for staging + activation of artifacts. Implemented by storage. Staging
// keeps an artifact in a temporary area until its full hash is verified, so an
// incomplete or corrupted artifact is never exposed as active.
class IArtifactStore {
   public:
    virtual ~IArtifactStore() = default;

    virtual edge_sync::base::Result<void> stage_chunk(const edge_sync::base::ArtifactId& id,
                                                      const edge_sync::base::Version& version,
                                                      const ChunkDescriptor& expected,
                                                      const edge_sync::base::ByteBuffer& data) = 0;

    virtual edge_sync::base::Result<void> commit(const Manifest& manifest) = 0;
    virtual edge_sync::base::Result<std::optional<edge_sync::base::Version>> active_version(
        const edge_sync::base::ArtifactId& id) = 0;
};

}  // namespace edge_sync::core
