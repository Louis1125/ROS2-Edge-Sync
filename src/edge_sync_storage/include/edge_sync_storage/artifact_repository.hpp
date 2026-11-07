#pragma once

#include "edge_sync_core/repository_interfaces.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

namespace edge_sync::storage {

// Implements core::IArtifactRepository by composing the SQLite metadata store
// with the content-addressed object store. The runtime injects this concrete
// implementation where core logic only sees the interface.
class ArtifactRepository final : public edge_sync::core::IArtifactRepository {
   public:
    ArtifactRepository(SqliteRepository& sqlite, ObjectStore& objects);

    edge_sync::base::Result<void> store(const edge_sync::core::Manifest& manifest) override;
    edge_sync::base::Result<edge_sync::core::Manifest> load(
        const edge_sync::base::ArtifactId& artifact_id) override;
    edge_sync::base::Result<bool> exists(const edge_sync::base::ArtifactId& artifact_id) override;

   private:
    SqliteRepository& sqlite_;
    ObjectStore& objects_;
};

}  // namespace edge_sync::storage
