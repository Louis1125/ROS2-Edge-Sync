#pragma once

#include <rclcpp_lifecycle/lifecycle_node.hpp>

namespace edge_sync::ros2 {

class LifecycleNode : public rclcpp_lifecycle::LifecycleNode {
public:
    explicit LifecycleNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
};

}  // namespace edge_sync::ros2
