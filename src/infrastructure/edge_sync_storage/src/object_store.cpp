#include "edge_sync_storage/object_store.hpp"

namespace edge_sync::storage {

edge_sync::base::Result<void> ObjectStore::put(const edge_sync::base::Hash&,
                                               const edge_sync::base::ByteBuffer&) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure, "not implemented"};
}

edge_sync::base::Result<edge_sync::base::ByteBuffer> ObjectStore::get(
    const edge_sync::base::Hash&) {
    return edge_sync::base::Error{edge_sync::base::ErrorCode::StorageFailure, "not implemented"};
}

}  // namespace edge_sync::storage
