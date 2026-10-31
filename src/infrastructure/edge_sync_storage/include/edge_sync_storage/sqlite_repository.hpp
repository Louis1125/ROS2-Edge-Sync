#pragma once

#include <string>

#include "edge_sync_base/result.hpp"

namespace edge_sync::storage {

class SqliteRepository {
public:
    explicit SqliteRepository(std::string path);

    edge_sync::base::Result<void> open();
    edge_sync::base::Result<void> close();

private:
    std::string path_;
};

}  // namespace edge_sync::storage
