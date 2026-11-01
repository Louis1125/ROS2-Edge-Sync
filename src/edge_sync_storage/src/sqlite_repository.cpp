#include "edge_sync_storage/sqlite_repository.hpp"

#include <filesystem>
#include <string>

namespace edge_sync::storage {
namespace {

constexpr const char* kSchema = R"SQL(
CREATE TABLE IF NOT EXISTS manifests (
  artifact_id TEXT PRIMARY KEY,
  version_major INTEGER NOT NULL,
  version_minor INTEGER NOT NULL,
  version_patch INTEGER NOT NULL,
  schema_version INTEGER NOT NULL,
  total_size INTEGER NOT NULL,
  chunk_size INTEGER NOT NULL,
  artifact_hash TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS manifest_chunks (
  artifact_id TEXT NOT NULL,
  chunk_index INTEGER NOT NULL,
  chunk_offset INTEGER NOT NULL,
  chunk_size INTEGER NOT NULL,
  chunk_hash TEXT NOT NULL
);
)SQL";

edge_sync::base::Error sql_error(sqlite3* db, const char* context) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                  std::string(context) + ": " +
                                      (db ? sqlite3_errmsg(db) : "unknown error")};
}

}  // namespace

SqliteRepository::SqliteRepository(std::string path) : path_(std::move(path)) {}

SqliteRepository::~SqliteRepository() { close(); }

edge_sync::base::Result<void> SqliteRepository::open() {
    if (db_) return {};

    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path_).parent_path(), ec);
    if (ec) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "cannot create database directory: " + ec.message()};
    }

    if (sqlite3_open(path_.c_str(), &db_) != SQLITE_OK) {
        const auto err = sql_error(db_, "cannot open database");
        db_ = nullptr;
        return err;
    }

    char* errmsg = nullptr;
    if (sqlite3_exec(db_, kSchema, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        const auto err = sql_error(db_, "cannot create schema");
        sqlite3_free(errmsg);
        sqlite3_close(db_);
        db_ = nullptr;
        return err;
    }
    return {};
}

edge_sync::base::Result<void> SqliteRepository::close() {
    if (!db_) return {};
    sqlite3_close(db_);
    db_ = nullptr;
    return {};
}

edge_sync::base::Result<void> SqliteRepository::store_manifest(
    const edge_sync::core::Manifest& manifest) {
    sqlite3_exec(db_, "BEGIN", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "INSERT OR REPLACE INTO manifests(artifact_id, version_major, version_minor, "
        "version_patch, schema_version, total_size, chunk_size, artifact_hash) "
        "VALUES(?,?,?,?,?,?,?,?)";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        return sql_error(db_, "prepare store manifest");
    }
    sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, static_cast<int>(manifest.version.major));
    sqlite3_bind_int(stmt, 3, static_cast<int>(manifest.version.minor));
    sqlite3_bind_int(stmt, 4, static_cast<int>(manifest.version.patch));
    sqlite3_bind_int(stmt, 5, static_cast<int>(manifest.schema_version));
    sqlite3_bind_int64(stmt, 6, static_cast<sqlite3_int64>(manifest.total_size));
    sqlite3_bind_int(stmt, 7, static_cast<int>(manifest.chunk_size));
    sqlite3_bind_text(stmt, 8, manifest.artifact_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    stmt = nullptr;
    sql = "DELETE FROM manifest_chunks WHERE artifact_id = ?";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    for (const auto& c : manifest.chunks) {
        stmt = nullptr;
        sql = "INSERT INTO manifest_chunks(artifact_id, chunk_index, chunk_offset, chunk_size, "
              "chunk_hash) VALUES(?,?,?,?,?)";
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(c.index));
        sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(c.offset));
        sqlite3_bind_int(stmt, 4, static_cast<int>(c.size));
        sqlite3_bind_text(stmt, 5, c.hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db_, "COMMIT", nullptr, nullptr, nullptr);
    return {};
}

edge_sync::base::Result<edge_sync::core::Manifest> SqliteRepository::load_manifest(
    const edge_sync::base::ArtifactId& artifact_id) {
    edge_sync::core::Manifest manifest;
    manifest.artifact_id = artifact_id;

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT version_major, version_minor, version_patch, schema_version, total_size, "
        "chunk_size, artifact_hash FROM manifests WHERE artifact_id = ?";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return sql_error(db_, "prepare load manifest");
    }
    sqlite3_bind_text(stmt, 1, artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "manifest not found: " + artifact_id};
    }
    manifest.version.major = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 0));
    manifest.version.minor = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 1));
    manifest.version.patch = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 2));
    manifest.schema_version = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 3));
    manifest.total_size = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 4));
    manifest.chunk_size = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 5));
    manifest.artifact_hash =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    sqlite3_finalize(stmt);

    stmt = nullptr;
    sql = "SELECT chunk_index, chunk_offset, chunk_size, chunk_hash FROM manifest_chunks "
          "WHERE artifact_id = ? ORDER BY chunk_index";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return sql_error(db_, "prepare load manifest chunks");
    }
    sqlite3_bind_text(stmt, 1, artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        edge_sync::core::ChunkDescriptor c;
        c.index = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 0));
        c.offset = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 1));
        c.size = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 2));
        c.hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        manifest.chunks.push_back(std::move(c));
    }
    sqlite3_finalize(stmt);
    return manifest;
}

edge_sync::base::Result<bool> SqliteRepository::manifest_exists(
    const edge_sync::base::ArtifactId& artifact_id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT 1 FROM manifests WHERE artifact_id = ?";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    const bool found = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return found;
}

}  // namespace edge_sync::storage
