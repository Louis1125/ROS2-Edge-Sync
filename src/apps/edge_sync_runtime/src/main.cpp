#include <rclcpp/rclcpp.hpp>

#include "edge_sync_runtime/application_builder.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);

    auto app = edge_sync::runtime::ApplicationBuilder{}.build(argc, argv);
    if (!app) {
        RCLCPP_ERROR(rclcpp::get_logger("edge_syncd"), "%s", app.error().message.c_str());
        rclcpp::shutdown();
        return 1;
    }

    app->run();
    rclcpp::shutdown();
    return 0;
}
