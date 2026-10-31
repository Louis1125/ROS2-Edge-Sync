#include "edge_sync_application/sync_coordinator.hpp"

namespace edge_sync::application {

SyncCoordinator::SyncCoordinator(edge_sync::connection::ConnectionManager& connections,
                                 edge_sync::domain::IArtifactRepository& repository)
    : connections_(connections), repository_(repository) {}

edge_sync::base::Result<void> SyncCoordinator::start_sync(
    const std::string&, const edge_sync::base::ArtifactId&) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::Internal, "not implemented"};
}

}  // namespace edge_sync::application
