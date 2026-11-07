#pragma once

#include "edge_sync_base/result.hpp"

namespace edge_sync::base {

// Common lifecycle for long-lived components. Components start in a
// non-running state, are activated, and must be shut down exactly once.
enum class LifecycleState { Inactive, Active, Shutdown };

class Lifecycle {
   public:
    virtual ~Lifecycle() = default;

    virtual Result<void> start() = 0;
    virtual Result<void> stop() = 0;

    [[nodiscard]] LifecycleState state() const noexcept {
        return state_;
    }

   protected:
    LifecycleState state_{LifecycleState::Inactive};
};

}  // namespace edge_sync::base
