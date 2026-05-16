// connection_interface.hpp
#pragma once

#include "common/network/socket_option.hpp"
#include "common/network/net_handler.hpp"
#include "close_function.hpp"
#include "common/core/net_state.hpp"
#include "common/core/data_option.hpp"

namespace lemondory::network {

// 네트워크 서버 인터페이스
class server {
public:
    virtual ~server() = default;

    virtual bool init(net_handler* handler, socket_option& option, int init_channel_count, int io_thread_count) = 0;
    virtual bool listen(const char* ip_address, int port) = 0;
    virtual bool stop() = 0;
    virtual void release() = 0;

    virtual void set_net_handler(net_handler* handler) = 0;
    virtual net_handler* get_net_handler() = 0;
};

// 서버 내부에서 관리되는 클라이언트 인터페이스
class server_client {
public:
    virtual ~server_client() = default;

    virtual bool init(net_handler* handler, socket_option& option) = 0;
    virtual bool connect(const char* remote_address, int port) = 0;
    virtual bool is_connected() = 0;
    virtual bool stop() = 0;
    virtual bool close(const void* error, close_function reason) = 0;

    virtual void set_socket_option(const socket_option& option) = 0;
    virtual void set_net_handler(net_handler* handler) = 0;

    virtual core::net_state get_state() = 0;
    virtual net_handler* get_net_handler() = 0;
};

// 일반 클라이언트 인터페이스
class client {
public:
    virtual ~client() = default;

    virtual bool connect(const char* remote_address, int port) = 0;
    virtual void connect_async(const char* remote_address, int port) = 0;
    virtual bool is_connected() = 0;
    virtual bool close(const void* error, close_function reason) = 0;

    virtual void set_socket_option(const socket_option& option) = 0;
    virtual void set_net_handler(net_handler* handler) = 0;
    virtual bool send(const char* buffer, std::uint32_t size, core::data_option option = core::data_option::none) = 0;
    virtual void tick() = 0;

    virtual core::net_state get_state() = 0;
    virtual void release() = 0;
    virtual net_handler* get_net_handler() = 0;
};

} // namespace lemondory::network