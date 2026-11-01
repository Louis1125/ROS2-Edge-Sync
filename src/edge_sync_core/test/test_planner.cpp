#include <gtest/gtest.h>

#include <optional>

#include "edge_sync_base/types.hpp"
#include "edge_sync_core/manifest_builder.hpp"
#include "edge_sync_core/sync_planner.hpp"

using edge_sync::base::ByteBuffer;
using edge_sync::core::build_manifest;
using edge_sync::core::SyncOutcome;
using edge_sync::core::SyncPlanner;

namespace {
ByteBuffer make_data(std::size_t n) {
    ByteBuffer b(n);
    for (std::size_t i = 0; i < n; ++i) b[i] = static_cast<std::uint8_t>(i % 251);
    return b;
}
}  // namespace

TEST(PlannerTest, NoLocalNeedsAllChunks) {
    auto remote = build_manifest("a", {1, 0, 0}, make_data(100), 32);
    ASSERT_TRUE(remote.ok());
    SyncPlanner planner;
    auto plan = planner.plan(std::nullopt, remote.value());
    ASSERT_TRUE(plan.ok());
    EXPECT_EQ(plan.value().outcome, SyncOutcome::Partial);
    EXPECT_EQ(plan.value().needed_chunks.size(), remote.value().chunks.size());
}

TEST(PlannerTest, IdenticalArtifactsNeedNothing) {
    auto local = build_manifest("a", {1, 0, 0}, make_data(100), 32);
    auto remote = build_manifest("a", {1, 0, 0}, make_data(100), 32);
    ASSERT_TRUE(local.ok());
    ASSERT_TRUE(remote.ok());
    SyncPlanner planner;
    auto plan = planner.plan(local.value(), remote.value());
    ASSERT_TRUE(plan.ok());
    EXPECT_EQ(plan.value().outcome, SyncOutcome::Identical);
    EXPECT_TRUE(plan.value().needed_chunks.empty());
}

TEST(PlannerTest, FetchesOnlyChangedChunk) {
    ByteBuffer base = make_data(100);
    ByteBuffer changed = base;
    changed[40] = 0xEE;  // mutates chunk index 1 (bytes 32..63)
    auto local = build_manifest("a", {1, 0, 0}, base, 32);
    auto remote = build_manifest("a", {1, 0, 0}, changed, 32);
    ASSERT_TRUE(local.ok());
    ASSERT_TRUE(remote.ok());

    SyncPlanner planner;
    auto plan = planner.plan(local.value(), remote.value());
    ASSERT_TRUE(plan.ok());
    EXPECT_EQ(plan.value().outcome, SyncOutcome::Partial);
    ASSERT_EQ(plan.value().needed_chunks.size(), 1u);
    EXPECT_EQ(plan.value().needed_chunks[0].index, 1u);
}

TEST(PlannerTest, VersionConflictWhenRemoteOlder) {
    auto local = build_manifest("a", {2, 0, 0}, make_data(100), 32);
    auto remote = build_manifest("a", {1, 0, 0}, make_data(100), 32);
    ASSERT_TRUE(local.ok());
    ASSERT_TRUE(remote.ok());
    SyncPlanner planner;
    auto plan = planner.plan(local.value(), remote.value());
    ASSERT_TRUE(plan.ok());
    EXPECT_EQ(plan.value().outcome, SyncOutcome::VersionConflict);
    EXPECT_TRUE(plan.value().needed_chunks.empty());
}

TEST(PlannerTest, DeduplicatesIdenticalChunks) {
    ByteBuffer d(64, 0x55);  // two identical 32-byte chunks
    auto remote = build_manifest("a", {1, 0, 0}, d, 32);
    ASSERT_TRUE(remote.ok());
    SyncPlanner planner;
    auto plan = planner.plan(std::nullopt, remote.value());
    ASSERT_TRUE(plan.ok());
    EXPECT_EQ(plan.value().needed_chunks.size(), 1u);
}
