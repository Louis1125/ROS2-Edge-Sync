#include <gtest/gtest.h>

#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest_builder.hpp"

using edge_sync::base::ByteBuffer;
using edge_sync::core::build_manifest;

TEST(ManifestBuilderTest, ChunkLayoutAndDistinctHashes) {
    ByteBuffer data(100, 0xAB);
    auto m = build_manifest("artifact-1", {1, 0, 0}, data, 32);
    ASSERT_TRUE(m.ok());

    EXPECT_EQ(m.value().total_size, 100u);
    EXPECT_EQ(m.value().chunk_size, 32u);
    ASSERT_EQ(m.value().chunks.size(), 4u);  // 32 + 32 + 32 + 4

    EXPECT_EQ(m.value().chunks[0].offset, 0u);
    EXPECT_EQ(m.value().chunks[0].size, 32u);
    EXPECT_EQ(m.value().chunks[3].offset, 96u);
    EXPECT_EQ(m.value().chunks[3].size, 4u);

    // The artifact hash is semantically distinct from every chunk hash.
    for (const auto& c : m.value().chunks) {
        EXPECT_NE(c.hash, m.value().artifact_hash);
    }
}

TEST(ManifestBuilderTest, SameContentSameHashes) {
    const ByteBuffer a{1, 2, 3, 4, 5};
    const ByteBuffer b{1, 2, 3, 4, 5};
    auto ma = build_manifest("x", {1, 0, 0}, a, 2);
    auto mb = build_manifest("x", {1, 0, 0}, b, 2);
    ASSERT_TRUE(ma.ok());
    ASSERT_TRUE(mb.ok());
    EXPECT_EQ(ma.value().artifact_hash, mb.value().artifact_hash);
    EXPECT_EQ(ma.value().chunks.size(), mb.value().chunks.size());
    EXPECT_EQ(ma.value().chunks[0].hash, mb.value().chunks[0].hash);
}

TEST(ManifestBuilderTest, RejectsZeroChunkSize) {
    ByteBuffer data{1, 2, 3};
    auto m = build_manifest("x", {1, 0, 0}, data, 0);
    ASSERT_FALSE(m.ok());
}
