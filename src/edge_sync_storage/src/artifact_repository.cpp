#include "edge_sync_storage/artifact_repository.hpp"

namespace edge_sync::storage {

ArtifactRepository::ArtifactRepository(SqliteRepository& sqlite, ObjectStore& objects)
    : sqlite_(sqlite), objects_(objects) {}

edge_sync::base::Result<void> ArtifactRepository::store(const edge_sync::core::Manifest& manifest) {
    for (const auto& chunk : manifest.chunks) {
        const auto ok = objects_.exists(chunk.hash);
        if (!ok.ok() || !ok.value()) {
            return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                          "missing object for chunk " +
                                              std::to_string(chunk.index)};
        }
    }
    return sqlite_.store_manifest(manifest);
}

edge_sync::base::Result<edge_sync::core::Manifest> ArtifactRepository::load(
    const edge_sync::base::ArtifactId& artifact_id) {
    return sqlite_.load_manifest(artifact_id);
}

edge_sync::base::Result<bool> ArtifactRepository::exists(
    const edge_sync::base::ArtifactId& artifact_id) {
    return sqlite_.manifest_exists(artifact_id);
}

}  // namespace edge_sync::storage
