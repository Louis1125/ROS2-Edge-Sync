#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::core {

// Builds a manifest from an artifact's bytes: chunks it, hashes each chunk and
// the full artifact. Never requires the whole artifact in memory beyond `data`.
edge_sync::base::Result<Manifest> build_manifest(const edge_sync::base::ArtifactId& artifact_id,
                                                 const edge_sync::base::Version& version,
                                                 const edge_sync::base::ByteBuffer& data,
                                                 std::uint32_t chunk_size);

}  // namespace edge_sync::core
