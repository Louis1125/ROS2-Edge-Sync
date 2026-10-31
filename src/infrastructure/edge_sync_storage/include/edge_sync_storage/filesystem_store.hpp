#pragma once

#include <string>

#include "edge_sync_base/result.hpp"
#include "edge_sync_base/types.hpp"

namespace edge_sync::storage {

class FilesystemStore {
public:
    explicit FilesystemStore(std::string root);

    edge_sync::base::Result<void> write(const edge_sync::base::Hash& key,
                                        const edge_sync::base::ByteBuffer& value);

private:
    std::string root_;
};

}  // namespace edge_sync::storage
