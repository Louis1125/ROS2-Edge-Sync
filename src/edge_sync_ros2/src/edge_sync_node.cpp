#include "edge_sync_ros2/edge_sync_node.hpp"

#include <chrono>

namespace edge_sync::ros2 {

EdgeSyncNode::EdgeSyncNode() : Node("edge_sync_node") {
    status_pub_ = create_publisher<edge_sync_interfaces::msg::SyncStatus>("edge_sync/status", 10);
    timer_ = create_wall_timer(std::chrono::seconds(1), [this] { publish_status(); });
}

void EdgeSyncNode::publish_status() {
    edge_sync_interfaces::msg::SyncStatus msg;
    msg.robot_id = "unknown";
    msg.state = "IDLE";
    msg.progress = 0;
    msg.error = "";
    status_pub_->publish(msg);
}

}  // namespace edge_sync::ros2
