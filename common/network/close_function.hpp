#pragma once

#include <cstdint>

namespace lemondory::network {

// 연결 종료 사유를 나타내는 열거형
enum class close_function : std::uint8_t {
    none = 0,
    write = 1,
    read = 2,
    timeout = 3,
    protocol_error = 4,
    remote_closed = 5,
    force_closed = 6
};

} // namespace lemondory::network
