#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "edge_sync_core/manifest_builder.hpp"
#include "edge_sync_storage/artifact_store.hpp"
#include "edge_sync_storage/object_store.hpp"
#include "edge_sync_storage/sqlite_repository.hpp"

using edge_sync::base::ByteBuffer;
using edge_sync::core::build_manifest;
using edge_sync::storage::ArtifactStore;
using edge_sync::storage::ObjectStore;
using edge_sync::storage::SqliteRepository;

namespace {
std::string unique_dir(const std::string& tag) {
    auto p = std::filesystem::temp_directory_path() / ("edge_sync_" + tag);
    std::filesystem::remove_all(p);
    std::filesystem::create_directories(p);
    return p.string();
}

ByteBuffer make_data(std::size_t n) {
    ByteBuffer b(n);
    for (std::size_t i = 0; i < n; ++i) b[i] = static_cast<std::uint8_t>(i * 13);
    return b;
}
}  // namespace

TEST(ArtifactStoreTest, StageCommitAndActivate) {
    const std::string dir = unique_dir("store");
    SqliteRepository sqlite(dir + "/meta.sqlite");
    ObjectStore objects(dir + "/objects");
    ArtifactStore store(dir + "/artifacts", objects, sqlite);
    ASSERT_TRUE(sqlite.open().ok());
    ASSERT_TRUE(objects.initialize().ok());
    ASSERT_TRUE(store.initialize().ok());

    const ByteBuffer data = make_data(100);
    auto manifest = build_manifest("map", {1, 0, 0}, data, 32);
    ASSERT_TRUE(manifest.ok());

    for (const auto& c : manifest.value().chunks) {
        ByteBuffer chunk(data.begin() + static_cast<std::ptrdiff_t>(c.offset),
                         data.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
        ASSERT_TRUE(store.stage_chunk("map", {1, 0, 0}, c, chunk).ok());
    }

    // Before commit, nothing is active.
    auto av = store.active_version("map");
    ASSERT_TRUE(av.ok());
    EXPECT_FALSE(av.value().has_value());

    ASSERT_TRUE(store.commit(manifest.value()).ok());
    av = store.active_version("map");
    ASSERT_TRUE(av.ok());
    ASSERT_TRUE(av.value().has_value());
    EXPECT_EQ(av.value()->major, 1u);
    EXPECT_TRUE(std::filesystem::exists(dir + "/artifacts/active/map"));
}

TEST(ArtifactStoreTest, RejectsCorruptedChunk) {
    const std::string dir = unique_dir("store_corrupt");
    SqliteRepository sqlite(dir + "/meta.sqlite");
    ObjectStore objects(dir + "/objects");
    ArtifactStore store(dir + "/artifacts", objects, sqlite);
    ASSERT_TRUE(sqlite.open().ok());
    ASSERT_TRUE(objects.initialize().ok());
    ASSERT_TRUE(store.initialize().ok());

    const ByteBuffer data = make_data(64);
    auto manifest = build_manifest("map", {1, 0, 0}, data, 32);
    ASSERT_TRUE(manifest.ok());

    ByteBuffer bad(32, 0xFF);  // wrong bytes
    ASSERT_FALSE(store.stage_chunk("map", {1, 0, 0}, manifest.value().chunks[0], bad).ok());
}

TEST(ArtifactStoreTest, IncompleteArtifactCannotCommit) {
    const std::string dir = unique_dir("store_incomplete");
    SqliteRepository sqlite(dir + "/meta.sqlite");
    ObjectStore objects(dir + "/objects");
    ArtifactStore store(dir + "/artifacts", objects, sqlite);
    ASSERT_TRUE(sqlite.open().ok());
    ASSERT_TRUE(objects.initialize().ok());
    ASSERT_TRUE(store.initialize().ok());

    const ByteBuffer data = make_data(100);  // 4 chunks
    auto manifest = build_manifest("map", {1, 0, 0}, data, 32);
    ASSERT_TRUE(manifest.ok());

    // stage only one chunk
    const auto& c = manifest.value().chunks[0];
    ByteBuffer chunk(data.begin() + static_cast<std::ptrdiff_t>(c.offset),
                     data.begin() + static_cast<std::ptrdiff_t>(c.offset + c.size));
    ASSERT_TRUE(store.stage_chunk("map", {1, 0, 0}, c, chunk).ok());

    ASSERT_FALSE(store.commit(manifest.value()).ok());
}
