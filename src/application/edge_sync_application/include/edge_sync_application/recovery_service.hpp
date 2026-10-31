#pragma once

#include "edge_sync_base/result.hpp"

namespace edge_sync::application {

class RecoveryService {
public:
    edge_sync::base::Result<void> recover();
};

}  // namespace edge_sync::application
