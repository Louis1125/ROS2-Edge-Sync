#pragma once

#include <cstdint>
#include <string>

namespace edge_sync::connection {

struct Endpoint {
    std::string address;
    std::uint16_t port{0};
};

}  // namespace edge_sync::connection
