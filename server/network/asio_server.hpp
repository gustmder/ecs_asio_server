#pragma once

#include <asio.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "common/network/socket_channel_base.hpp"
#include "common/network/net_handler.hpp"
#include "common/core/data_option.hpp"
#include "server/core/thread_group.hpp"
#include "common/network/socket_option.hpp"

namespace lemondory::network {

class asio_channel;
class message_dispatcher;

class asio_server : public std::enable_shared_from_this<asio_server> {
public:
    explicit asio_server(asio::io_context& io_context);
    ~asio_server();

    bool init(net_handler* handler, const socket_option& option, int init_channel_count, int io_thread_count);
    bool listen(const std::string& ip_address, int port);
    bool stop();
    void release();

    void set_net_handler(net_handler* handler);
    net_handler* get_net_handler();

    using on_frame_callback = std::function<void(int, std::uint16_t, const char*, std::size_t)>;
    void set_on_frame(on_frame_callback callback) { on_frame_ = std::move(callback); }

private:
    void do_accept();
    void handle_accept(std::shared_ptr<asio_channel> channel, const asio::error_code& ec);

private:
    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
    net_handler* handler_ = nullptr;
    socket_option socket_option_;
    std::atomic<bool> running_ = false;

    std::mutex channel_mutex_;
    std::unordered_map<int, std::shared_ptr<asio_channel>> channels_;

    core::thread_group thread_group_;

    // 채널 ID 발급용 - 원자적 연산으로 변경
    std::atomic<int> next_channel_id_{1};

    on_frame_callback on_frame_;
};

} // namespace lemondory::network