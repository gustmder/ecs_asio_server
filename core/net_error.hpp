#pragma once

#include <cstdint>

namespace lemondory::core {

// 네트워크 에러 코드 정의
enum class net_error : std::uint8_t {
    ok = 0,                  // 성공
    invalid_socket,          // 잘못된 소켓
    invalid_address,         // 주소 오류
    timeout,                 // 타임아웃
    send_buffer_full,        // 송신 버퍼 가득 참
    send_already_closed,     // 이미 닫힌 상태에서 전송 시도
    recv_buffer_full,        // 수신 버퍼 가득 참
    send_invalid_param,      // 잘못된 전송 인자
    io_pending,              // I/O 진행 중
    connect_failed           // 연결 실패
};

} // namespace lemondory::core
