#pragma once

#include <cstdint>
#include <string>

#include "edge_sync_base/types.hpp"

namespace edge_sync::base {

// Incremental SHA-256. Feed bytes with update(); call final_hex() once.
class Sha256 {
public:
    Sha256();

    void update(const std::uint8_t* data, std::size_t size);
    Hash final_hex();

private:
    void transform_block(const std::uint8_t* block);

    std::uint32_t h_[8];
    std::uint8_t buffer_[64];
    std::size_t buffer_len_{0};
    std::uint64_t total_len_{0};
};

// Convenience one-shot helpers.
Hash sha256_hex(const std::uint8_t* data, std::size_t size);
Hash sha256_hex(const ByteBuffer& data);
Hash sha256_hex(const std::string& data);
// Hash a file in blocks without loading it into memory.
Hash sha256_hex_file(const std::string& path);

}  // namespace edge_sync::base
