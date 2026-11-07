#include "edge_sync_base/hmac.hpp"

#include <array>

#include "edge_sync_base/sha256.hpp"

namespace edge_sync::base {
namespace {

std::string hex_to_bytes(const std::string& hex) {
    auto val = [](char c) -> int {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    };
    std::string out(hex.size() / 2, '\0');
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        out[i / 2] = static_cast<char>((val(hex[i]) << 4) | val(hex[i + 1]));
    }
    return out;
}

}  // namespace

Hash hmac_sha256(const std::string& key, const std::string& message) {
    std::string k = key;
    if (k.size() > 64) {
        k = sha256_hex(k);
    }
    k.resize(64, '\0');

    std::array<std::uint8_t, 64> ipad{};
    std::array<std::uint8_t, 64> opad{};
    for (std::size_t i = 0; i < 64; ++i) {
        ipad[i] = static_cast<std::uint8_t>(k[i]) ^ 0x36;
        opad[i] = static_cast<std::uint8_t>(k[i]) ^ 0x5c;
    }

    Sha256 inner;
    inner.update(ipad.data(), ipad.size());
    inner.update(reinterpret_cast<const std::uint8_t*>(message.data()), message.size());
    const std::string inner_bytes = hex_to_bytes(inner.final_hex());

    Sha256 outer;
    outer.update(opad.data(), opad.size());
    outer.update(reinterpret_cast<const std::uint8_t*>(inner_bytes.data()), inner_bytes.size());
    return outer.final_hex();
}

}  // namespace edge_sync::base
