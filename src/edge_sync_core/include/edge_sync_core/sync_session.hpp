#pragma once

#include <string>

#include "edge_sync_base/types.hpp"

namespace edge_sync::core {

enum class SyncState { Planning, Transferring, Verifying, Activating, Complete, Failed };

struct SyncSession {
    std::string session_id;
    std::string source_robot_id;
    std::string target_robot_id;
    edge_sync::base::ArtifactId artifact_id;
    SyncState state{SyncState::Planning};
};

}  // namespace edge_sync::core
