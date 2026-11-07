#include "edge_sync_connection/tcp_transport.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/write.hpp>
#include <future>
#include <utility>

namespace edge_sync::connection {
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

TcpTransport::TcpTransport(TimeoutConfig timeouts)
    : timeouts_(timeouts), work_guard_(boost::asio::make_work_guard(io_)) {}

TcpTransport::~TcpTransport() {
    stop();
}

void TcpTransport::ensure_io_thread() {
    if (!io_thread_.joinable()) {
        io_thread_ = std::jthread([this] { io_.run(); });
    }
}

edge_sync::base::Result<void> TcpTransport::start(const Endpoint& listen_endpoint) {
    if (stopped_) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                      "transport already stopped"};
    }

    tcp::endpoint ep(asio::ip::make_address(listen_endpoint.address), listen_endpoint.port);
    acceptor_ = std::make_unique<tcp::acceptor>(io_);
    boost::system::error_code ec;
    acceptor_->open(ep.protocol(), ec);
    if (!ec)
        acceptor_->set_option(tcp::acceptor::reuse_address(true), ec);
    if (!ec)
        acceptor_->bind(ep, ec);
    if (!ec)
        acceptor_->listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        acceptor_.reset();
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed, ec.message()};
    }

    ensure_io_thread();
    accept_loop();
    return {};
}

void TcpTransport::accept_loop() {
    if (stopped_)
        return;
    acceptor_->async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
        if (stopped_ || ec)
            return;

        auto conn = std::make_shared<Conn>();
        conn->socket = std::make_shared<tcp::socket>(std::move(socket));
        const ConnectionId id = next_id_++;
        conns_[id] = conn;

        boost::system::error_code remote_ec;
        const auto remote = conn->socket->remote_endpoint(remote_ec);
        const Endpoint endpoint{remote.address().to_string(), remote.port()};

        if (callbacks_.on_accepted)
            callbacks_.on_accepted(id, endpoint);
        start_read(id, conn);
        accept_loop();
    });
}

edge_sync::base::Result<ConnectionId> TcpTransport::connect(const Endpoint& endpoint) {
    if (stopped_) {
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                      "transport already stopped"};
    }
    ensure_io_thread();

    auto socket = std::make_shared<tcp::socket>(io_);
    auto conn = std::make_shared<Conn>();
    conn->socket = socket;
    const ConnectionId id = next_id_++;
    conns_[id] = conn;

    tcp::resolver resolver(io_);
    boost::system::error_code ec;
    const auto results = resolver.resolve(endpoint.address, std::to_string(endpoint.port), ec);
    if (ec) {
        conns_.erase(id);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed, ec.message()};
    }

    auto prom = std::make_shared<std::promise<boost::system::error_code>>();
    auto fut = prom->get_future();
    asio::async_connect(
        *socket, results,
        [prom](const boost::system::error_code& e, const tcp::endpoint&) { prom->set_value(e); });

    if (fut.wait_for(timeouts_.connect) != std::future_status::ready) {
        boost::system::error_code ignore;
        socket->close(ignore);
        conns_.erase(id);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::Timeout, "connect timed out"};
    }

    const boost::system::error_code result = fut.get();
    if (result) {
        conns_.erase(id);
        return edge_sync::base::Error{edge_sync::base::ErrorCode::ConnectionFailed,
                                      result.message()};
    }

    start_read(id, conn);
    return id;
}

void TcpTransport::start_read(ConnectionId id, const std::shared_ptr<Conn>& conn) {
    auto buf = std::make_shared<ByteBuffer>(4096);
    conn->socket->async_read_some(
        asio::buffer(*buf),
        [this, id, conn, buf](const boost::system::error_code& ec, std::size_t size) {
            if (ec) {
                conns_.erase(id);
                if (callbacks_.on_disconnected)
                    callbacks_.on_disconnected(id);
                return;
            }
            std::vector<ByteBuffer> frames;
            conn->decoder.feed(buf->data(), size, frames);
            for (auto& frame : frames) {
                if (callbacks_.on_message)
                    callbacks_.on_message(id, std::move(frame));
            }
            start_read(id, conn);
        });
}

void TcpTransport::send(ConnectionId id, const ByteBuffer& data) {
    const auto it = conns_.find(id);
    if (it == conns_.end())
        return;

    auto framed = std::make_shared<ByteBuffer>(encode_frame(data));
    asio::async_write(*it->second->socket, asio::buffer(*framed),
                      [framed](const boost::system::error_code&, std::size_t) {
                          // Errors surface via the read loop's on_disconnected.
                      });
}

void TcpTransport::close(ConnectionId id) {
    const auto it = conns_.find(id);
    if (it == conns_.end())
        return;
    boost::system::error_code ec;
    it->second->socket->shutdown(tcp::socket::shutdown_both, ec);
    it->second->socket->close(ec);
    conns_.erase(id);
}

void TcpTransport::stop() {
    if (stopped_.exchange(true))
        return;

    if (acceptor_) {
        boost::system::error_code ec;
        acceptor_->close(ec);
        acceptor_.reset();
    }
    work_guard_.reset();
    io_.stop();
    if (io_thread_.joinable())
        io_thread_.join();

    for (auto& [id, conn] : conns_) {
        boost::system::error_code ec;
        if (conn->socket->is_open())
            conn->socket->close(ec);
    }
    conns_.clear();
}

}  // namespace edge_sync::connection
