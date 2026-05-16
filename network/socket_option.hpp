#pragma once

#include <cstdint>

namespace lemondory::network {

// 소켓 생성 시 적용할 설정들을 캡슐화한 옵션 클래스
class socket_option {
public:
    bool reuse_address() const { return reuse_address_; }
    bool tcp_no_delay() const { return tcp_no_delay_; }
    bool linger() const { return linger_; }
    std::int32_t backlog() const { return backlog_; }
    std::int32_t connect_timeout() const { return connect_timeout_; }

    socket_option& set_linger(bool enable) {
        linger_ = enable;
        return *this;
    }

    socket_option& set_reuse_address(bool enable) {
        reuse_address_ = enable;
        return *this;
    }

    socket_option& set_tcp_no_delay(bool enable) {
        tcp_no_delay_ = enable;
        return *this;
    }

    socket_option& set_backlog(std::int32_t backlog) {
        backlog_ = backlog;
        return *this;
    }

    socket_option& set_connect_timeout(std::int32_t timeout) {
        connect_timeout_ = timeout;
        return *this;
    }

private:
    bool reuse_address_ = false;
    bool tcp_no_delay_ = false;
    bool linger_ = false;
    std::int32_t backlog_ = 128;
    std::int32_t connect_timeout_ = 5;
};

} // namespace lemondory::network