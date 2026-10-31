#pragma once

#include <rclcpp/rclcpp.hpp>

#include "edge_sync_interfaces/msg/sync_status.hpp"

namespace edge_sync::ros2 {

// ROS 2 adapter node. In M5 this will drive the core/connection foundation via
// actions and services; for now it publishes a heartbeat status message.
class EdgeSyncNode : public rclcpp::Node {
public:
    EdgeSyncNode();

private:
    void publish_status();

    rclcpp::Publisher<edge_sync_interfaces::msg::SyncStatus>::SharedPtr status_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace edge_sync::ros2
