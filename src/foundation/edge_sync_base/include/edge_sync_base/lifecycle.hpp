#pragma once

#include "edge_sync_base/result.hpp"

namespace edge_sync::base {

enum class LifecycleState { Unconfigured, Inactive, Active, Finalized };

class Lifecycle {
public:
    virtual ~Lifecycle() = default;

    virtual Result<void> configure() = 0;
    virtual Result<void> activate() = 0;
    virtual Result<void> deactivate() = 0;
    virtual Result<void> cleanup() = 0;
    virtual Result<void> shutdown() = 0;
};

}  // namespace edge_sync::base
