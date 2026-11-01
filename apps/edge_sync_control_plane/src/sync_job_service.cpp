#include "edge_sync_control_plane/application/sync_job_service.hpp"

namespace edge_sync::control_plane {
std::atomic<std::uint64_t> SyncJobService::seq_{0};
}  // namespace edge_sync::control_plane
