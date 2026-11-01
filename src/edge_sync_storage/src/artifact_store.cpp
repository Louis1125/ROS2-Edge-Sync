#include "edge_sync_storage/artifact_store.hpp"

#include <filesystem>
#include <fstream>

#include "edge_sync_base/sha256.hpp"

namespace edge_sync::storage {
namespace fs = std::filesystem;

ArtifactStore::ArtifactStore(std::string root, ObjectStore& objects, SqliteRepository& sqlite)
    : root_(std::move(root)), objects_(objects), sqlite_(sqlite) {}

edge_sync::base::Result<void> ArtifactStore::initialize() {
    std::error_code ec;
    fs::create_directories(root_ + "/staging", ec);
    fs::create_directories(root_ + "/active", ec);
    if (ec) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "cannot create artifact store: " + ec.message()};
    }
    return {};
}

void ArtifactStore::reset_staging(const edge_sync::base::ArtifactId& id,
                                  const edge_sync::base::Version& version) {
    staging_id_ = id;
    staging_version_ = version;
    staged_bytes_ = 0;
    staged_indices_.clear();
    std::ofstream(staging_path(), std::ios::binary | std::ios::trunc).close();
}

edge_sync::base::Result<void> ArtifactStore::stage_chunk(
    const edge_sync::base::ArtifactId& id, const edge_sync::base::Version& version,
    const edge_sync::core::ChunkDescriptor& expected, const edge_sync::base::ByteBuffer& data) {
    if (staging_id_ != id || !(staging_version_.major == version.major &&
                               staging_version_.minor == version.minor &&
                               staging_version_.patch == version.patch)) {
        reset_staging(id, version);
    }
    if (staged_indices_.count(expected.index)) return {};  // idempotent duplicate

    if (edge_sync::base::sha256_hex(data) != expected.hash) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                      "chunk " + std::to_string(expected.index) +
                                          " hash mismatch"};
    }
    if (auto r = objects_.put(data); !r) return r.error();

    std::fstream out(staging_path(), std::ios::in | std::ios::out | std::ios::binary);
    if (!out) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "cannot open staging file"};
    }
    out.seekp(static_cast<std::streamoff>(expected.offset));
    out.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
    out.flush();
    if (!out) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "staging write failed"};
    }

    staged_indices_.insert(expected.index);
    staged_bytes_ += data.size();
    return {};
}

edge_sync::base::Result<void> ArtifactStore::commit(const edge_sync::core::Manifest& manifest) {
    if (staging_id_ != manifest.artifact_id) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                      "no staged artifact for " + manifest.artifact_id};
    }
    if (staged_bytes_ != manifest.total_size) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                      "incomplete artifact: " + std::to_string(staged_bytes_) +
                                          " of " + std::to_string(manifest.total_size) + " bytes"};
    }
    if (edge_sync::base::sha256_hex_file(staging_path()) != manifest.artifact_hash) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::IntegrityFailure,
                                      "artifact hash mismatch"};
    }

    std::error_code ec;
    fs::rename(staging_path(), active_path(manifest.artifact_id), ec);
    if (ec) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure,
                                      "atomic activation failed: " + ec.message()};
    }
    if (auto r = sqlite_.store_active_version(manifest.artifact_id, manifest.version); !r) {
        return r.error();
    }
    staging_id_.clear();
    staged_bytes_ = 0;
    staged_indices_.clear();
    return {};
}

edge_sync::base::Result<std::optional<edge_sync::base::Version>> ArtifactStore::active_version(
    const edge_sync::base::ArtifactId& id) {
    return sqlite_.load_active_version(id);
}

std::string ArtifactStore::staging_path() const { return root_ + "/staging/" + staging_id_; }

std::string ArtifactStore::active_path(const edge_sync::base::ArtifactId& id) const {
    return root_ + "/active/" + id;
}

}  // namespace edge_sync::storage
