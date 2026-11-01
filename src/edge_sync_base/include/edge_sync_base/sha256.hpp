#pragma once

#include <cstdint>
#include <string>

#include "edge_sync_base/types.hpp"

namespace edge_sync::base {

// SHA-256 digest of a byte buffer, hex-encoded (lowercase).
Hash sha256_hex(const std::uint8_t* data, std::size_t size);
Hash sha256_hex(const ByteBuffer& data);
Hash sha256_hex(const std::string& data);

}  // namespace edge_sync::base
