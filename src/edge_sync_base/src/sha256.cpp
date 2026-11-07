#include "edge_sync_base/sha256.hpp"

#include <array>
#include <cstring>
#include <fstream>
#include <vector>

namespace edge_sync::base {
namespace {

constexpr std::array<std::uint32_t, 64> kRound = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

std::uint32_t rotr(std::uint32_t x, std::uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

std::string hex(const std::uint8_t* data, std::size_t n) {
    static const char* digits = "0123456789abcdef";
    std::string out(n * 2, '0');
    for (std::size_t i = 0; i < n; ++i) {
        out[i * 2] = digits[data[i] >> 4];
        out[i * 2 + 1] = digits[data[i] & 0xf];
    }
    return out;
}

}  // namespace

Sha256::Sha256()
    : h_{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
         0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19} {}

void Sha256::transform_block(const std::uint8_t* block) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<std::uint32_t>(block[i * 4]) << 24) |
               (static_cast<std::uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<std::uint32_t>(block[i * 4 + 2]) << 8) |
               static_cast<std::uint32_t>(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
        const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    std::uint32_t a = h_[0], b = h_[1], c = h_[2], d = h_[3];
    std::uint32_t e = h_[4], f = h_[5], g = h_[6], hh = h_[7];
    for (int i = 0; i < 64; ++i) {
        const std::uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t t1 = hh + s1 + ch + kRound[i] + w[i];
        const std::uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = s0 + maj;
        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    h_[0] += a;
    h_[1] += b;
    h_[2] += c;
    h_[3] += d;
    h_[4] += e;
    h_[5] += f;
    h_[6] += g;
    h_[7] += hh;
}

void Sha256::update(const std::uint8_t* data, std::size_t size) {
    total_len_ += size;
    const std::uint8_t* p = data;
    std::size_t remaining = size;

    if (buffer_len_ > 0) {
        const std::size_t to_fill = 64 - buffer_len_;
        const std::size_t take = std::min(to_fill, remaining);
        std::memcpy(buffer_ + buffer_len_, p, take);
        buffer_len_ += take;
        p += take;
        remaining -= take;
        if (buffer_len_ == 64) {
            transform_block(buffer_);
            buffer_len_ = 0;
        }
    }
    while (remaining >= 64) {
        transform_block(p);
        p += 64;
        remaining -= 64;
    }
    if (remaining > 0) {
        std::memcpy(buffer_, p, remaining);
        buffer_len_ = remaining;
    }
}

Hash Sha256::final_hex() {
    const std::uint64_t bit_len = total_len_ * 8;
    std::uint8_t tail[128] = {0};
    std::memcpy(tail, buffer_, buffer_len_);
    tail[buffer_len_] = 0x80;
    const std::size_t padded = ((buffer_len_ + 8) / 64 + 1) * 64;
    for (int i = 0; i < 8; ++i) {
        tail[padded - 1 - i] = static_cast<std::uint8_t>((bit_len >> (8 * i)) & 0xff);
    }
    for (std::size_t off = 0; off < padded; off += 64) {
        transform_block(tail + off);
    }

    std::uint8_t digest[32];
    for (int i = 0; i < 8; ++i) {
        digest[i * 4] = static_cast<std::uint8_t>((h_[i] >> 24) & 0xff);
        digest[i * 4 + 1] = static_cast<std::uint8_t>((h_[i] >> 16) & 0xff);
        digest[i * 4 + 2] = static_cast<std::uint8_t>((h_[i] >> 8) & 0xff);
        digest[i * 4 + 3] = static_cast<std::uint8_t>(h_[i] & 0xff);
    }
    return hex(digest, 32);
}

Hash sha256_hex(const std::uint8_t* data, std::size_t size) {
    Sha256 h;
    h.update(data, size);
    return h.final_hex();
}

Hash sha256_hex(const ByteBuffer& data) {
    return sha256_hex(data.data(), data.size());
}

Hash sha256_hex(const std::string& data) {
    return sha256_hex(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

Hash sha256_hex_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    Sha256 h;
    std::array<std::uint8_t, 1 << 20> buf{};
    while (in) {
        in.read(reinterpret_cast<char*>(buf.data()), buf.size());
        const auto n = in.gcount();
        if (n > 0)
            h.update(buf.data(), static_cast<std::size_t>(n));
    }
    return h.final_hex();
}

}  // namespace edge_sync::base
