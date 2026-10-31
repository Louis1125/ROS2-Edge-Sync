#pragma once

#include <cstdint>
#include <string>

#include "edge_sync_base/types.hpp"

namespace edge_sync::storage {

// SHA-256 digest of a byte buffer, hex-encoded (lowercase).
edge_sync::base::Hash sha256_hex(const std::uint8_t* data, std::size_t size);
edge_sync::base::Hash sha256_hex(const edge_sync::base::ByteBuffer& data);
edge_sync::base::Hash sha256_hex(const std::string& data);

}  // namespace edge_sync::storage
