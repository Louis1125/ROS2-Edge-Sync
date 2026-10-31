#pragma once

#include <chrono>
#include <cstdint>

namespace edge_sync::connection {

struct RetryPolicy {
    std::uint32_t max_attempts{5};
    std::chrono::milliseconds base_delay{100};
    std::chrono::milliseconds max_delay{5000};
    double backoff_factor{2.0};
};

}  // namespace edge_sync::connection
