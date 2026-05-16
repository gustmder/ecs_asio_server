#pragma once

#include <cstdint>

namespace lemondory::core {

// 채널 또는 소켓의 연결 상태를 나타내는 열거형
enum class net_state : std::uint8_t {
    closed = 0,     // 닫힘 (초기 상태 또는 연결 종료됨)
    connecting,     // 연결 중 (비동기 연결 시도 중)
    connected       // 연결 완료됨
};

} // namespace lemondory::core