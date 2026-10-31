#pragma once

#include <chrono>

namespace edge_sync::connection {

struct TimeoutConfig {
    std::chrono::milliseconds connect{5000};
    std::chrono::milliseconds handshake{5000};
    std::chrono::milliseconds idle{60000};
};

}  // namespace edge_sync::connection
