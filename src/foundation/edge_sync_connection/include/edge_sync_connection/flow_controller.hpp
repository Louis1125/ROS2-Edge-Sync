#pragma once

#include <cstddef>

namespace edge_sync::connection {

class FlowController {
public:
    explicit FlowController(std::size_t max_inflight = 16) : max_inflight_(max_inflight) {}

    [[nodiscard]] std::size_t max_inflight() const noexcept { return max_inflight_; }

private:
    std::size_t max_inflight_;
};

}  // namespace edge_sync::connection
