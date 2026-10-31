#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "edge_sync_storage/artifact_repository.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

using edge_sync::core::FileEntry;
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
    auto hash = objects.put(content);
    ASSERT_TRUE(hash.ok());

    Manifest m;
    m.artifact_id = "map-2024";
    m.version = {1, 2, 0};
    m.files.push_back(FileEntry{"grid.pgm", hash.value(), content.size(), 1});

    ASSERT_TRUE(repo.store(m).ok());
    ASSERT_TRUE(repo.exists("map-2024").value());

    auto loaded = repo.load("map-2024");
    ASSERT_TRUE(loaded.ok());
    EXPECT_EQ(loaded.value().artifact_id, "map-2024");
    ASSERT_EQ(loaded.value().files.size(), 1u);
    EXPECT_EQ(loaded.value().files[0].hash, hash.value());

    sqlite.close();
}
