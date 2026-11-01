#pragma once

#include <cstdint>
#include <string>

namespace edge_sync::control_plane {

struct SyncJob {
    std::string job_id;
    std::string source_robot;
    std::string target_robot;
    std::string artifact_id;
    std::string status;           // queued | transferring | completed | failed | cancelled
    std::uint64_t transferred_bytes{0};
    std::uint64_t total_bytes{0};
    std::string error;
};

}  // namespace edge_sync::control_plane
