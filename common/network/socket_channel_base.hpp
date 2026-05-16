// socket_channel_base.hpp
#pragma once

#include "common/network/net_handler.hpp"
#include "common/core/net_state.hpp"
#include "common/core/data_option.hpp"

#include <cstdint>
#include <functional>

namespace lemondory::network {

using socket_receive_func = std::function<void(const char*, std::int32_t)>;
using socket_close_func = std::function<void(const void*, close_function)>;

class socket_channel_base {
public:
    socket_channel_base();
    virtual ~socket_channel_base();

    virtual void set_channel_id(int channel_id) {channel_id_ = channel_id;};
    int get_channel_id() const {return channel_id_; }
    virtual bool is_closed() const { return closed_; }

    void set_recv_callback(socket_receive_func callback) {recv_callback_ = std::move(callback); }
    void set_close_callback(socket_close_func callback) {close_callback_ = std::move(callback); }

    virtual void on_close(const void* error, close_function close_function);
    virtual void on_connect(net_error net_error);
    virtual void on_accept();

    virtual void send(const char* data, int size) = 0;
    virtual void recv(char* buffer, int size) = 0;
    virtual bool close(const void* error, close_function reason) = 0;

protected:
    int channel_id_ = -1;
    net_handler* handler_ = nullptr;

    bool closed_ = false;
    bool closing_ = false;

    socket_receive_func recv_callback_ = nullptr;
    socket_close_func close_callback_ = nullptr;
};

} // namespace lemondory::network
