#include <gtest/gtest.h>

#include <string>

#include "edge_sync_base/hmac.hpp"

using edge_sync::base::hmac_sha256;

TEST(HmacTest, Rfc4231Vector) {
    // RFC 4231 test case 1.
    const std::string key = "key";
    const std::string msg = "The quick brown fox jumps over the lazy dog";
    EXPECT_EQ(hmac_sha256(key, msg),
              "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

TEST(HmacTest, DeterministicAndKeySensitive) {
    const std::string msg = "robot-a";
    EXPECT_EQ(hmac_sha256("secret", msg), hmac_sha256("secret", msg));
    EXPECT_NE(hmac_sha256("secret", msg), hmac_sha256("other", msg));
}
