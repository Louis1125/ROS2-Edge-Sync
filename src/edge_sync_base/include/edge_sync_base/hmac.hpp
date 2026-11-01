#pragma once

#include <string>

#include "edge_sync_base/types.hpp"

namespace edge_sync::base {

// HMAC-SHA256 (RFC 2104), hex-encoded. Used to authenticate the robot
// handshake with a shared secret.
Hash hmac_sha256(const std::string& key, const std::string& message);

}  // namespace edge_sync::base
