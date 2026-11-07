#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "edge_sync_ros2/edge_sync_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<edge_sync::ros2::EdgeSyncNode>());
    rclcpp::shutdown();
    return 0;
}
