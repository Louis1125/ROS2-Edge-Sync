#pragma once

#include <cstdint>
#include <string>

namespace edge_sync::control_plane {

struct RobotRecord {
    std::string robot_id;
    std::string name;
    std::string status;  // online | offline
    std::string runtime_version;
    std::int64_t last_seen_ms{0};
};

}  // namespace edge_sync::control_plane
