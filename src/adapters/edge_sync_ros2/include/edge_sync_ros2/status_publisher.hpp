#pragma once

#include <rclcpp/rclcpp.hpp>

#include "edge_sync_interfaces/msg/sync_status.hpp"

namespace edge_sync::ros2 {

class StatusPublisher {
public:
    explicit StatusPublisher(rclcpp::Node::SharedPtr node);

    void publish(const edge_sync_interfaces::msg::SyncStatus& status);

private:
    rclcpp::Publisher<edge_sync_interfaces::msg::SyncStatus>::SharedPtr publisher_;
};

}  // namespace edge_sync::ros2
