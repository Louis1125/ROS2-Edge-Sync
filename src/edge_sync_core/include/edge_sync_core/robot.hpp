#pragma once

#include <string>

namespace edge_sync::core {

struct Robot {
    std::string robot_id;
    std::string name;
    std::string role;  // "robot" | "coordinator"
};

}  // namespace edge_sync::core
