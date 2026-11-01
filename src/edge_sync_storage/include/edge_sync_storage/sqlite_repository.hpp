#pragma once

#include <string>

#include <sqlite3.h>

#include "edge_sync_base/result.hpp"
#include <optional>

#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest.hpp"

namespace edge_sync::storage {

// SQLite-backed manifest metadata repository. Owns the connection; must be
// opened before use and closed exactly once.
class SqliteRepository {
public:
    explicit SqliteRepository(std::string path);
    ~SqliteRepository();

    SqliteRepository(const SqliteRepository&) = delete;
    SqliteRepository& operator=(const SqliteRepository&) = delete;

    edge_sync::base::Result<void> open();
    edge_sync::base::Result<void> close();

    edge_sync::base::Result<void> store_manifest(const edge_sync::core::Manifest& manifest);
    edge_sync::base::Result<edge_sync::core::Manifest> load_manifest(
        const edge_sync::base::ArtifactId& artifact_id);
    edge_sync::base::Result<bool> manifest_exists(const edge_sync::base::ArtifactId& artifact_id);

    edge_sync::base::Result<void> store_active_version(
        const edge_sync::base::ArtifactId& artifact_id, const edge_sync::base::Version& version);
    edge_sync::base::Result<std::optional<edge_sync::base::Version>> load_active_version(
        const edge_sync::base::ArtifactId& artifact_id);

private:
    std::string path_;
    sqlite3* db_{nullptr};
};

}  // namespace edge_sync::storage
