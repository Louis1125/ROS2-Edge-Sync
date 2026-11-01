#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "edge_sync_base/types.hpp"

namespace edge_sync::core {

// Metadata for one fixed-size chunk of an artifact. `hash` is the SHA-256 of
// the chunk bytes — distinct from the artifact-level hash.
struct ChunkDescriptor {
    std::uint64_t index{0};
    std::uint64_t offset{0};
    std::uint32_t size{0};
    edge_sync::base::Hash hash;
};

// Versioned, content-addressed description of an artifact as a list of chunks.
// `schema_version` lets the manifest format evolve; `artifact_hash` is the
// SHA-256 of the full artifact bytes.
struct Manifest {
    edge_sync::base::ArtifactId artifact_id;
    edge_sync::base::Version version;
    std::uint32_t schema_version{1};
    std::uint64_t total_size{0};
    std::uint32_t chunk_size{0};
    edge_sync::base::Hash artifact_hash;
    std::vector<ChunkDescriptor> chunks;
};

}  // namespace edge_sync::core
