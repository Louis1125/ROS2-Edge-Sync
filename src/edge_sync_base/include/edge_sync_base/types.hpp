#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace edge_sync::base {

using RobotId = std::string;
using ArtifactId = std::string;
using Hash = std::string;  // hex-encoded digest
using ByteBuffer = std::vector<std::uint8_t>;

struct Version {
    std::uint32_t major{0};
    std::uint32_t minor{0};
    std::uint32_t patch{0};
};

}  // namespace edge_sync::base
