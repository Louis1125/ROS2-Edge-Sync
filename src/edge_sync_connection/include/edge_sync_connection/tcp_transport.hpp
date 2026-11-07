#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <thread>

#include "edge_sync_connection/frame_codec.hpp"
#include "edge_sync_connection/timeout_manager.hpp"
#include "edge_sync_connection/transport_interface.hpp"

namespace edge_sync::connection {

// Asio TCP transport. Runs a single-threaded I/O loop (so all callbacks are
// serialized and ConnectionManager needs no internal locking). Messages are
// length-prefixed (see FrameDecoder).
class TcpTransport final : public ITransport {
   public:
    explicit TcpTransport(TimeoutConfig timeouts = {});
    ~TcpTransport() override;

    TcpTransport(const TcpTransport&) = delete;
    TcpTransport& operator=(const TcpTransport&) = delete;

    edge_sync::base::Result<void> start(const Endpoint& listen_endpoint) override;
    edge_sync::base::Result<ConnectionId> connect(const Endpoint& endpoint) override;
    void send(ConnectionId id, const ByteBuffer& data) override;
    void close(ConnectionId id) override;
    void stop() override;

   private:
    struct Conn {
        std::shared_ptr<boost::asio::ip::tcp::socket> socket;
        FrameDecoder decoder;
    };

    void ensure_io_thread();
    void accept_loop();
    void start_read(ConnectionId id, const std::shared_ptr<Conn>& conn);

    boost::asio::io_context io_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::map<ConnectionId, std::shared_ptr<Conn>> conns_;
    ConnectionId next_id_{1};
    std::jthread io_thread_;
    std::atomic<bool> stopped_{false};
    TimeoutConfig timeouts_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
};

}  // namespace edge_sync::connection
