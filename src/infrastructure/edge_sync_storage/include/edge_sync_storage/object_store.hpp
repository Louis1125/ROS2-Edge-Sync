#pragma once

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::storage {

class ObjectStore {
public:
    edge_sync::base::Result<void> put(const edge_sync::base::Hash& key,
                                      const edge_sync::base::ByteBuffer& value);
    edge_sync::base::Result<edge_sync::base::ByteBuffer> get(
        const edge_sync::base::Hash& key);
};

}  // namespace edge_sync::storage
