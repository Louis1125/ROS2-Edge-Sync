#include <gtest/gtest.h>

#include "edge_sync_core/version.hpp"

using edge_sync::base::Version;
using edge_sync::core::compare_versions;
using edge_sync::core::VersionOrder;

TEST(VersionTest, OrdersBySemver) {
    EXPECT_EQ(compare_versions({1, 0, 0}, {1, 0, 0}), VersionOrder::Equal);
    EXPECT_EQ(compare_versions({1, 0, 0}, {1, 1, 0}), VersionOrder::Older);
    EXPECT_EQ(compare_versions({2, 0, 0}, {1, 9, 9}), VersionOrder::Newer);
    EXPECT_EQ(compare_versions({1, 0, 1}, {1, 0, 0}), VersionOrder::Newer);
}
