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
  version_patch INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS manifest_files (
  artifact_id TEXT NOT NULL,
  path TEXT NOT NULL,
  hash TEXT NOT NULL,
  size INTEGER NOT NULL,
  chunk_count INTEGER NOT NULL
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
        "version_patch) VALUES(?,?,?,?)";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK", nullptr, nullptr, nullptr);
        return sql_error(db_, "prepare store manifest");
    }
    sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, static_cast<int>(manifest.version.major));
    sqlite3_bind_int(stmt, 3, static_cast<int>(manifest.version.minor));
    sqlite3_bind_int(stmt, 4, static_cast<int>(manifest.version.patch));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    stmt = nullptr;
    sql = "DELETE FROM manifest_files WHERE artifact_id = ?";
    sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    for (const auto& f : manifest.files) {
        stmt = nullptr;
        sql = "INSERT INTO manifest_files(artifact_id, path, hash, size, chunk_count) "
              "VALUES(?,?,?,?,?)";
        sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, manifest.artifact_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, f.path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, f.hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 4, static_cast<sqlite3_int64>(f.size));
        sqlite3_bind_int(stmt, 5, static_cast<int>(f.chunk_count));
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
    const char* sql = "SELECT version_major, version_minor, version_patch FROM manifests "
                      "WHERE artifact_id = ?";
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
    sqlite3_finalize(stmt);

    stmt = nullptr;
    sql = "SELECT path, hash, size, chunk_count FROM manifest_files WHERE artifact_id = ? "
          "ORDER BY rowid";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return sql_error(db_, "prepare load manifest files");
    }
    sqlite3_bind_text(stmt, 1, artifact_id.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        edge_sync::core::FileEntry f;
        f.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        f.hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        f.size = static_cast<std::uint64_t>(sqlite3_column_int64(stmt, 2));
        f.chunk_count = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 3));
        manifest.files.push_back(std::move(f));
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
