#include "edge_sync_runtime/shutdown_handler.hpp"

#include <csignal>

#include <rclcpp/rclcpp.hpp>

namespace edge_sync::runtime {
namespace {
void on_signal(int) { rclcpp::shutdown(); }
}  // namespace

void install_shutdown_handlers() {
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);
}

}  // namespace edge_sync::runtime
