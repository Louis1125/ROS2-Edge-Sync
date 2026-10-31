#include <gtest/gtest.h>

#include "edge_sync_base/result.hpp"

using edge_sync::base::Error;
using edge_sync::base::ErrorCode;
using edge_sync::base::Result;

TEST(ResultTest, OkHoldsValue) {
    Result<int> r{42};
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.value(), 42);
}

TEST(ResultTest, ErrorHoldsCode) {
    Result<int> r{Error{ErrorCode::Timeout, "timed out"}};
    ASSERT_FALSE(r.ok());
    EXPECT_EQ(r.error().code, ErrorCode::Timeout);
}

TEST(ResultVoidTest, DefaultIsOk) {
    Result<void> r;
    ASSERT_TRUE(r.ok());
}
