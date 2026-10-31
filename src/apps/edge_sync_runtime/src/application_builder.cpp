#include "edge_sync_runtime/application_builder.hpp"

#include <memory>

#include <rclcpp/rclcpp.hpp>

#include "edge_sync_runtime/shutdown_handler.hpp"

namespace edge_sync::runtime {

void Application::run() {
    // Spin the ROS 2 executor until shutdown is requested.
    rclcpp::spin(std::make_shared<rclcpp::Node>("edge_sync"));
}

edge_sync::base::Result<std::unique_ptr<Application>> ApplicationBuilder::build(int, char**) {
    // Composition root: wire config, logger, storage, connection manager,
    // transport, sync coordinator and ROS 2 nodes here (M1+).
    install_shutdown_handlers();
    return std::make_unique<Application>();
}

}  // namespace edge_sync::runtime
