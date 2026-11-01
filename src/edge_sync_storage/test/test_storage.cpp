#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "edge_sync_core/manifest_builder.hpp"
#include "edge_sync_storage/artifact_repository.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

using edge_sync::core::Manifest;
using edge_sync::storage::ArtifactRepository;
using edge_sync::storage::ObjectStore;
using edge_sync::storage::SqliteRepository;

namespace {
std::string unique_dir(const std::string& tag) {
    auto p = std::filesystem::temp_directory_path() / ("edge_sync_" + tag);
    std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);
    return p.string();
}
}  // namespace

TEST(ObjectStoreTest, PutGetRoundTripAndDedup) {
    ObjectStore store(unique_dir("objects"));
    ASSERT_TRUE(store.initialize().ok());

    edge_sync::base::ByteBuffer data{0xde, 0xad, 0xbe, 0xef};
    auto hash = store.put(data);
    ASSERT_TRUE(hash.ok());

    ASSERT_TRUE(store.exists(hash.value()).value());
    auto got = store.get(hash.value());
    ASSERT_TRUE(got.ok());
    EXPECT_EQ(got.value(), data);

    // Identical content yields the same hash (content addressing).
    auto hash2 = store.put(data);
    ASSERT_TRUE(hash2.ok());
    EXPECT_EQ(hash2.value(), hash.value());
}

TEST(ArtifactRepositoryTest, StoreAndLoadManifest) {
    const std::string dir = unique_dir("repo");
    SqliteRepository sqlite(dir + "/meta.sqlite");
    ObjectStore objects(dir + "/objects");
    ASSERT_TRUE(sqlite.open().ok());
    ASSERT_TRUE(objects.initialize().ok());

    ArtifactRepository repo(sqlite, objects);

    edge_sync::base::ByteBuffer content{'m', 'a', 'p'};
    auto manifest = edge_sync::core::build_manifest("map-2024", {1, 2, 0}, content, 2);
    ASSERT_TRUE(manifest.ok());
    for (const auto& c : manifest.value().chunks) {
        edge_sync::base::ByteBuffer chunk(
            content.begin() + static_cast<std::ptrdiff_t>(c.offset),
            content.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
        ASSERT_TRUE(objects.put(chunk).ok());
    }

    ASSERT_TRUE(repo.store(manifest.value()).ok());
    ASSERT_TRUE(repo.exists("map-2024").value());

    auto loaded = repo.load("map-2024");
    ASSERT_TRUE(loaded.ok());
    EXPECT_EQ(loaded.value().artifact_id, "map-2024");
    ASSERT_EQ(loaded.value().chunks.size(), manifest.value().chunks.size());
    EXPECT_EQ(loaded.value().artifact_hash, manifest.value().artifact_hash);

    sqlite.close();
}
