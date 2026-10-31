#include "edge_sync_connection/connection_manager.hpp"

namespace edge_sync::connection {

ConnectionManager::ConnectionManager(std::unique_ptr<ITransport> transport)
    : transport_(std::move(transport)) {}

edge_sync::base::Result<void> ConnectionManager::start() { return {}; }

edge_sync::base::Result<Session::Id> ConnectionManager::open_session(const Peer&) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::Internal, "not implemented"};
}

edge_sync::base::Result<void> ConnectionManager::close_session(Session::Id) { return {}; }

edge_sync::base::Result<void> ConnectionManager::shutdown() { return {}; }

}  // namespace edge_sync::connection
