#include "edge_sync_core/manifest_builder.hpp"

#include "edge_sync_base/sha256.hpp"

namespace edge_sync::core {

edge_sync::base::Result<Manifest> build_manifest(const edge_sync::base::ArtifactId& artifact_id,
                                                 const edge_sync::base::Version& version,
                                                 const edge_sync::base::ByteBuffer& data,
                                                 std::uint32_t chunk_size) {
    if (chunk_size == 0) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::InvalidArgument,
                                      "chunk size must be non-zero"};
    }

    Manifest manifest;
    manifest.artifact_id = artifact_id;
    manifest.version = version;
    manifest.schema_version = 1;
    manifest.total_size = data.size();
    manifest.chunk_size = chunk_size;
    manifest.artifact_hash = edge_sync::base::sha256_hex(data);

    std::uint64_t index = 0;
    for (std::size_t offset = 0; offset < data.size(); offset += chunk_size, ++index) {
        const std::size_t size = std::min<std::size_t>(chunk_size, data.size() - offset);
        edge_sync::base::ByteBuffer chunk(
            data.begin() + static_cast<std::ptrdiff_t>(offset),
            data.begin() + static_cast<std::ptrdiff_t>(offset + size));
        manifest.chunks.push_back(ChunkDescriptor{index, offset, static_cast<std::uint32_t>(size),
                                                  edge_sync::base::sha256_hex(chunk)});
    }
    return manifest;
}

}  // namespace edge_sync::core
