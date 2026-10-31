#include <gtest/gtest.h>

#include "edge_sync_protocol/message_codec.hpp"

using edge_sync::protocol::MessageEnvelope;
using edge_sync::protocol::kProtocolVersion;

TEST(ProtocolTest, HandshakeRoundTrip) {
    MessageEnvelope in;
    in.set_robot_id("robot-a");
    in.set_protocol_version(kProtocolVersion);
    in.mutable_handshake_request()->set_robot_id("robot-a");
    in.mutable_handshake_request()->set_protocol_version(kProtocolVersion);

    auto bytes = edge_sync::protocol::serialize(in);
    ASSERT_TRUE(bytes.ok());

    auto out = edge_sync::protocol::parse(bytes.value());
    ASSERT_TRUE(out.ok());
    EXPECT_EQ(out.value().robot_id(), "robot-a");
    ASSERT_TRUE(out.value().has_handshake_request());
    EXPECT_EQ(out.value().handshake_request().robot_id(), "robot-a");
}

TEST(ProtocolTest, ParseGarbageFails) {
    edge_sync::base::ByteBuffer garbage{0xde, 0xad, 0xbe, 0xef};
    auto out = edge_sync::protocol::parse(garbage);
    ASSERT_FALSE(out.ok());
    EXPECT_EQ(out.error().code, edge_sync::base::ErrorCode::ProtocolMismatch);
}
