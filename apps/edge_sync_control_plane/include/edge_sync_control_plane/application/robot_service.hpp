#pragma once

#include <vector>

#include "edge_sync_base/result.hpp"
#include "edge_sync_control_plane/domain/robot.hpp"
#include "edge_sync_control_plane/infrastructure/database.hpp"

namespace edge_sync::control_plane {

class RobotService {
   public:
    explicit RobotService(Database& db) : db_(db) {}

    edge_sync::base::Result<std::vector<RobotRecord>> list() {
        return db_.list_robots();
    }

   private:
    Database& db_;
};

}  // namespace edge_sync::control_plane
