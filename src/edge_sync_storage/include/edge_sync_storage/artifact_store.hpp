#pragma once

#include <optional>
#include <set>
#include <string>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"
#include "edge_sync_core/artifact_store.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

namespace edge_sync::storage {

// Stages chunks into a temporary file, verifies chunk + full-artifact hashes,
// then atomically renames the staged file into place. Records the active
// version in SQLite. An artifact is never visible as active until commit().
class ArtifactStore final : public edge_sync::core::IArtifactStore {
   public:
    ArtifactStore(std::string root, ObjectStore& objects, SqliteRepository& sqlite);

    edge_sync::base::Result<void> initialize();

    edge_sync::base::Result<void> stage_chunk(const edge_sync::base::ArtifactId& id,
                                              const edge_sync::base::Version& version,
                                              const edge_sync::core::ChunkDescriptor& expected,
                                              const edge_sync::base::ByteBuffer& data) override;
    edge_sync::base::Result<void> commit(const edge_sync::core::Manifest& manifest) override;
    edge_sync::base::Result<std::optional<edge_sync::base::Version>> active_version(
        const edge_sync::base::ArtifactId& id) override;

   private:
    [[nodiscard]] std::string staging_path() const;
    [[nodiscard]] std::string active_path(const edge_sync::base::ArtifactId& id) const;
    void reset_staging(const edge_sync::base::ArtifactId& id, const edge_sync::base::Version& v);

    std::string root_;
    ObjectStore& objects_;
    SqliteRepository& sqlite_;

    edge_sync::base::ArtifactId staging_id_;
    edge_sync::base::Version staging_version_;
    std::uint64_t staged_bytes_{0};
    std::set<std::uint64_t> staged_indices_;
};

}  // namespace edge_sync::storage
