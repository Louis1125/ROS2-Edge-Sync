#pragma once

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include "edge_sync_interfaces/action/sync_artifact.hpp"

namespace edge_sync::ros2 {

class SyncActionServer {
public:
    explicit SyncActionServer(rclcpp::Node::SharedPtr node);

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp_action::Server<edge_sync_interfaces::action::SyncArtifact>::SharedPtr server_;
};

}  // namespace edge_sync::ros2
