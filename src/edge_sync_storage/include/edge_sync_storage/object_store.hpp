#pragma once

#include <string>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::storage {

// Content-addressed object store on the local filesystem. Objects are written
// once under <root>/<aa>/<full-hash> and are immutable, so identical content is
// stored exactly once.
class ObjectStore {
public:
    explicit ObjectStore(std::string root);

    edge_sync::base::Result<void> initialize();
    edge_sync::base::Result<edge_sync::base::Hash> put(const edge_sync::base::ByteBuffer& data);
    edge_sync::base::Result<edge_sync::base::ByteBuffer> get(const edge_sync::base::Hash& hash);
    edge_sync::base::Result<bool> exists(const edge_sync::base::Hash& hash);

private:
    [[nodiscard]] std::string path_for(const edge_sync::base::Hash& hash) const;

    std::string root_;
};

}  // namespace edge_sync::storage
