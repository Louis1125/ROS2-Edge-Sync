#pragma once

#include <rclcpp/rclcpp.hpp>

namespace edge_sync::ros2 {

class EdgeSyncNode : public rclcpp::Node {
public:
    EdgeSyncNode();
};

}  // namespace edge_sync::ros2
