#pragma once

#include <string>

#include "edge_sync_base/result.hpp"

namespace edge_sync::storage {

class AtomicActivation {
public:
    edge_sync::base::Result<void> activate(const std::string& staged_dir,
                                           const std::string& live_dir);
};

}  // namespace edge_sync::storage
