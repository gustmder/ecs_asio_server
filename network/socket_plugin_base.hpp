#pragma once

#include "message.hpp"

namespace lemondory::network {

// 소켓 메시지를 처리하기 위한 플러그인 인터페이스
class socket_plugin_base {
public:
    virtual ~socket_plugin_base() = default;

    // 메시지를 처리하고 성공 여부 반환
    virtual bool process(message* msg) = 0;

    // 내부 상태 초기화
    virtual void reset() = 0;
};

} // namespace lemondory::network