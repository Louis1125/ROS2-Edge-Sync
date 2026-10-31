#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "edge_sync_base/config.hpp"

using edge_sync::base::Config;

namespace {
std::string write_temp(const std::string& content) {
    const std::string path = "/tmp/edge_sync_config_test.yaml";
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}
}  // namespace

TEST(ConfigTest, ParsesNestedKeys) {
    const auto path = write_temp(
        "robot:\n"
        "  id: robot-a\n"
        "  role: robot\n"
        "connection:\n"
        "  port: 7443\n"
        "  tls:\n"
        "    enabled: true\n"
        "  retry:\n"
        "    max_attempts: 5\n");

    auto cfg = Config::load_file(path);
    ASSERT_TRUE(cfg.ok());
    EXPECT_EQ(cfg.value().get_string("robot.id"), "robot-a");
    EXPECT_EQ(cfg.value().get_int("connection.port"), 7443);
    EXPECT_TRUE(cfg.value().get_bool("connection.tls.enabled"));
    EXPECT_EQ(cfg.value().get_int("connection.retry.max_attempts"), 5);
    std::remove(path.c_str());
}

TEST(ConfigTest, MissingFileIsError) {
    auto cfg = Config::load_file("/tmp/does_not_exist_12345.yaml");
    ASSERT_FALSE(cfg.ok());
    EXPECT_EQ(cfg.error().code, edge_sync::base::ErrorCode::InvalidArgument);
}
