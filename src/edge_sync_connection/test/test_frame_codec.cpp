#include <gtest/gtest.h>

#include <vector>

#include "edge_sync_connection/frame_codec.hpp"

using edge_sync::connection::ByteBuffer;
using edge_sync::connection::encode_frame;
using edge_sync::connection::FrameDecoder;

TEST(FrameCodecTest, RoundTrip) {
    const ByteBuffer payload{1, 2, 3, 4, 5};
    const auto frame = encode_frame(payload);
    ASSERT_EQ(frame.size(), payload.size() + 4);

    FrameDecoder d;
    std::vector<ByteBuffer> out;
    d.feed(frame.data(), frame.size(), out);
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], payload);
}

TEST(FrameCodecTest, ByteAtATime) {
    ByteBuffer payload(100, 7);
    const auto frame = encode_frame(payload);

    FrameDecoder d;
    std::vector<ByteBuffer> out;
    for (std::size_t i = 0; i < frame.size(); ++i) {
        d.feed(&frame[i], 1, out);
    }
    ASSERT_EQ(out.size(), 1u);
    EXPECT_EQ(out[0], payload);
}

TEST(FrameCodecTest, TwoFramesInOneFeed) {
    const ByteBuffer a{1, 2}, b{3, 4, 5, 6};
    ByteBuffer combined = encode_frame(a);
    const auto fb = encode_frame(b);
    combined.insert(combined.end(), fb.begin(), fb.end());

    FrameDecoder d;
    std::vector<ByteBuffer> out;
    d.feed(combined.data(), combined.size(), out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], a);
    EXPECT_EQ(out[1], b);
}
