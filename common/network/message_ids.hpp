#pragma once
#include <cstdint>

// namespace lemondory::protocol {

// // English: Well-known message IDs shared by client/server.
// // 한국어: 클라이언트/서버 공통 메시지 ID 상수.
// inline constexpr std::uint16_t msg_ping = 1;
// inline constexpr std::uint16_t msg_pong = 2;
// inline constexpr std::uint16_t msg_echo = 100;

// // English: frame limits (hard guard)
// // 한국어: 프레임 상한(하드 가드)
// inline constexpr std::uint32_t max_frame_bytes = 1u << 20; // 1MB

// // v1 extended header bytes (after len)
// // len(4) + [cmd(2),flags(1),ver(1),seq(4),crc32(4)] = 4 + 12
// inline constexpr std::size_t header_ext_after_len = 12;
// inline constexpr std::size_t header_ext_total     = 4 + header_ext_after_len; // 16

// } // namespace lemondory::protocol

// network/message_ids.hpp

// offset  size  name      설명
// 0       4     len       header+payload 길이 (== 16 + payload)
// 4       2     cmd       메시지 ID
// 6       1     flags     0x01 압축, 0x02 암호화, 0x04 서브스트림 … (일단 0)
// 7       1     ver       프로토콜 버전 (지금은 1)
// 8       4     seq       전송 시퀀스 (지금은 0 or 증가값)
// 12      4     crc32     payload CRC32 (지금은 0 or 선택 적용)
// 16      N     payload

#pragma once
#include <cstdint>

namespace lemondory::network {

// 명령 ID(Command) — 고정폭 enum (type-safe)
// [EXTENSION POINT] 새로운 메시지가 생기면 여기에 항목을 추가하세요.
enum class command : std::uint16_t {
    ping = 1,
    pong = 2,
    echo = 100,
    // add more...
};

// enum → 원시값 변환 헬퍼
constexpr std::uint16_t to_u16(command c) noexcept {
    return static_cast<std::uint16_t>(c);
}

// Flags (예약/확장 필드)
// [EXTENSION POINT] 압축/암호화/서브스트림 등 추가 시 플래그 비트만 예약해두고 나중에 사용하세요.
inline constexpr std::uint8_t flag_compress  = 0x01; // compression
inline constexpr std::uint8_t flag_encrypt   = 0x02; // encryption
inline constexpr std::uint8_t flag_substream = 0x04; // substream

// Protocol version
// [EXTENSION POINT] 스키마 변경 시 버전 관리에 사용합니다.
inline constexpr std::uint8_t proto_ver = 1;

} // namespace lemondory::network