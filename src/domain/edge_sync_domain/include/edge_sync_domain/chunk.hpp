#pragma once

#include <cstdint>

#include "edge_sync_base/types.hpp"

namespace edge_sync::domain {

struct Chunk {
    edge_sync::base::ByteBuffer data;
    std::uint64_t index{0};
    std::uint64_t offset{0};
    std::uint32_t length{0};
};

}  // namespace edge_sync::domain
