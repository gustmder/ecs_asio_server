#pragma once
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>
#include <stdexcept>

namespace lemondory::network::frame_codec 
{
    // ---- proto constants (v1.5 header) ----
    inline constexpr std::uint8_t  k_proto_ver        = 1;
    inline constexpr std::size_t   k_header_after_len = 12;   // cmd(2)+flags(1)+ver(1)+seq(4)+crc(4)
    inline constexpr std::size_t   k_header_total     = 16;   // len(4) + 12
    inline constexpr std::size_t   k_max_frame_bytes  = (1u<<20); // 1MB

    // ---- little-endian helpers ----
    inline void put_le16(char* p, std::uint16_t v) {
        p[0] = (char)(v & 0xFF);
        p[1] = (char)((v >> 8) & 0xFF);
    }
    inline void put_le32(char* p, std::uint32_t v) {
        p[0] = (char)(v & 0xFF);
        p[1] = (char)((v >> 8) & 0xFF);
        p[2] = (char)((v >> 16) & 0xFF);
        p[3] = (char)((v >> 24) & 0xFF);
    }
    inline std::uint16_t get_le16(const char* p) {
        return  static_cast<std::uint16_t>((std::uint8_t)p[0])
            | (static_cast<std::uint16_t>((std::uint8_t)p[1]) << 8);
    }
    inline std::uint32_t get_le32(const char* p) {
        return  (std::uint8_t)p[0]
            | ((std::uint32_t)(std::uint8_t)p[1] << 8)
            | ((std::uint32_t)(std::uint8_t)p[2] << 16)
            | ((std::uint32_t)(std::uint8_t)p[3] << 24);
    }

    // ---- tiny CRC32 (IEEE) for payload ----
    inline std::uint32_t crc32(const char* data, std::size_t len) {
        static std::uint32_t table[256];
        static bool inited = false;
        if (!inited) {
            for (std::uint32_t i = 0; i < 256; ++i) {
                std::uint32_t c = i;
                for (int k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
                table[i] = c;
            }
            inited = true;
        }
        std::uint32_t c = 0xFFFFFFFFu;
        for (std::size_t i = 0; i < len; ++i)
            c = table[(c ^ (std::uint8_t)data[i]) & 0xFFu] ^ (c >> 8);
        return c ^ 0xFFFFFFFFu;
    }

    // ---- header structure (after len) ----
    struct header_v1 {
        std::uint16_t cmd{};
        std::uint8_t  flags{};
        std::uint8_t  ver{ k_proto_ver };
        std::uint32_t seq{};
        std::uint32_t crc{};
    };

    // build: len(4)=12+payload + header_v1 + payload
    inline std::vector<char> pack(std::uint16_t cmd,
                                std::uint8_t flags,
                                std::uint8_t ver,
                                std::uint32_t seq,
                                std::string_view payload)
    {
        const std::uint32_t len_after_len = static_cast<std::uint32_t>(k_header_after_len + payload.size());
        std::vector<char> out(k_header_total + payload.size());

        // len
        put_le32(out.data(), len_after_len);

        // cmd, flags, ver, seq
        put_le16(out.data() + 4, cmd);
        out[6] = (char)flags;
        out[7] = (char)ver;
        put_le32(out.data() + 8, seq);

        // payload
        if (!payload.empty()) {
            std::memcpy(out.data() + 16, payload.data(), payload.size());
        }

        // crc over payload
        const std::uint32_t c = crc32(out.data() + 16, payload.size());
        put_le32(out.data() + 12, c);

        return out;
    }

    // parse one frame from accumulator `acc`
    // returns: true if one frame consumed (even if CRC invalid), outputs header & payload view
    inline bool try_parse_one(std::vector<char>& acc, header_v1& out_hdr, std::string_view& out_payload) {
        // need len
        if (acc.size() < 4) return false;

        const std::uint32_t len_after_len = get_le32(acc.data());
        if (len_after_len > k_max_frame_bytes) {
            acc.clear(); // desync → drop
            return false;
        }
        const std::size_t total = 4u + (std::size_t)len_after_len;
        if (acc.size() < total) return false; // incomplete

        if (len_after_len < k_header_after_len) {
            acc.clear(); // invalid header
            return false;
        }

        // header
        out_hdr.cmd   = get_le16(acc.data() + 4);
        out_hdr.flags = (std::uint8_t)acc[6];
        out_hdr.ver   = (std::uint8_t)acc[7];
        out_hdr.seq   = get_le32(acc.data() + 8);
        out_hdr.crc   = get_le32(acc.data() + 12);

        // payload view
        const char* payload_ptr = acc.data() + 16;
        const std::size_t payload_sz = (std::size_t)len_after_len - k_header_after_len;
        out_payload = std::string_view(payload_ptr, payload_sz);

        // crc check
        const std::uint32_t c = crc32(payload_ptr, payload_sz);
        // consume frame regardless (잘못된 CRC라도 프레임은 제거)
        acc.erase(acc.begin(), acc.begin() + (std::ptrdiff_t)total);

        // NOTE: 필요 시 여기서 CRC 오류 시 close 처리할 수 있음.
        (void)c; // 호출부 정책에 따라 사용
        return true;
    }

} // namespace lemondory::network::frame_codec




// #pragma once
// #include <cstdint>
// #include <cstring>
// #include <vector>
// #include <stdexcept>

// namespace lemondory::network {

// // ---- 프로토콜 상수 (v1.5 확장 헤더) ----
// inline constexpr std::uint8_t  k_proto_ver          = 1;        // [확장포인트] 버전
// inline constexpr std::size_t   k_header_after_len   = 12;       // cmd(2)+flags(1)+ver(1)+seq(4)+crc(4) = 12
// inline constexpr std::size_t   k_header_total       = 16;       // len(4) + above(12) = 16
// inline constexpr std::size_t   k_max_frame_bytes    = (1u<<20); // 1MB 안전 상한

// // ---- 프레임 헤더 구조 ----
// struct frame_header {
//     std::uint32_t len;    // 전체 길이 = header(16) + payload
//     std::uint16_t cmd;    // 명령
//     std::uint8_t  flags;  // [확장포인트] 압축/암호화/서브스트림
//     std::uint8_t  ver;    // [확장포인트] 프로토콜 버전
//     std::uint32_t seq;    // [확장포인트] 시퀀스
//     std::uint32_t crc32;  // [확장포인트] payload CRC
// };

// // ---- 작은 LE 헬퍼 ----
// inline void put_le16(char* p, std::uint16_t v) { p[0]=char(v&0xFF); p[1]=char((v>>8)&0xFF); }
// inline void put_le32(char* p, std::uint32_t v) {
//     p[0]=char(v&0xFF); p[1]=char((v>>8)&0xFF); p[2]=char((v>>16)&0xFF); p[3]=char((v>>24)&0xFF);
// }
// inline std::uint16_t get_le16(const char* p) { return std::uint8_t(p[0]) | (std::uint16_t(std::uint8_t(p[1]))<<8); }
// inline std::uint32_t get_le32(const char* p) {
//     return  std::uint8_t(p[0])
//           | (std::uint32_t(std::uint8_t(p[1]))<<8)
//           | (std::uint32_t(std::uint8_t(p[2]))<<16)
//           | (std::uint32_t(std::uint8_t(p[3]))<<24);
// }

// // ---- 간단 CRC32 (payload 무결성; 필요 시 교체 가능) ----
// inline std::uint32_t crc32_ieee(const char* data, std::size_t len) {
//     static std::uint32_t table[256]; static bool init=false;
//     if (!init) {
//         for (std::uint32_t i=0;i<256;i++){ std::uint32_t c=i; for(int k=0;k<8;k++) c=(c&1)?(0xEDB88320u^(c>>1)):(c>>1); table[i]=c; }
//         init=true;
//     }
//     std::uint32_t c=0xFFFFFFFFu;
//     for (std::size_t i=0;i<len;i++) c = table[(c ^ std::uint8_t(data[i])) & 0xFFu] ^ (c >> 8);
//     return c ^ 0xFFFFFFFFu;
// }

// // ---- 프레이밍 클래스 (기존 사용법 유지) ----
// class frame_codec {
// public:
//     // pack: (cmd만 필수) 나머지는 확장포인트
//     static std::vector<char> pack(std::uint16_t cmd,
//                                   const void* payload, std::size_t size,
//                                   std::uint8_t flags = 0,
//                                   std::uint32_t seq = 0,
//                                   bool with_crc = false)
//     {
//         const std::uint32_t total = static_cast<std::uint32_t>(k_header_total + size);
//         std::vector<char> out(total);

//         // len(4)
//         put_le32(out.data()+0, total);
//         // cmd(2), flags(1), ver(1), seq(4)
//         put_le16(out.data()+4, cmd);
//         out[6] = char(flags);
//         out[7] = char(k_proto_ver);
//         put_le32(out.data()+8, seq);
//         // crc(4)
//         const std::uint32_t crc = (with_crc && payload && size) ? crc32_ieee(static_cast<const char*>(payload), size) : 0;
//         put_le32(out.data()+12, crc);

//         if (payload && size) std::memcpy(out.data()+k_header_total, payload, size);
//         return out;
//     }

//     // parse_header: 누적버퍼에서 헤더만 읽기
//     static frame_header parse_header(const char* buf, std::size_t buf_sz) {
//         if (buf_sz < k_header_total) throw std::runtime_error("short header");

//         frame_header h{};
//         h.len   = get_le32(buf+0);
//         h.cmd   = get_le16(buf+4);
//         h.flags = std::uint8_t(buf[6]);
//         h.ver   = std::uint8_t(buf[7]);
//         h.seq   = get_le32(buf+8);
//         h.crc32 = get_le32(buf+12);
//         return h;
//     }

//     // sanity helpers
//     static bool valid_total_len(std::uint32_t total) {
//         return total >= k_header_total && total <= k_max_frame_bytes;
//     }
// };

// } // namespace lemondory::network





// #pragma once
// #include <cstdint>
// #include <vector>
// #include <cstring>
// #include <stdexcept>
// #include "message_ids.hpp"

// namespace lemondory::network {

// // 16바이트 고정 헤더 (v1.5)
// // len = 16 + payload
// struct frame_header {
//     std::uint32_t len;   // total bytes = header(16) + payload
//     std::uint16_t cmd;   // command id
//     std::uint8_t  flags; // [EXTENSION POINT] compression/encryption/substream
//     std::uint8_t  ver;   // [EXTENSION POINT] protocol versioning
//     std::uint32_t seq;   // [EXTENSION POINT] sequence number (loss/ordering)
//     std::uint32_t crc32; // [EXTENSION POINT] payload integrity
// };

// // 기존 사용법 유지: frame_codec::pack(), frame_codec::parse_header()
// class frame_codec {
// public:
//     // Create frame (pack) — 현재는 cmd만 실사용, 나머지는 0/기본값
//     // [EXTENSION POINT] flags/seq/do_crc 파라미터를 나중에 실제로 사용하면
//     // 와이어 변경 없이 확장됩니다.
//     static std::vector<char> pack(std::uint16_t cmd,
//                                   const void* data,
//                                   std::size_t size,
//                                   std::uint8_t flags = 0,
//                                   std::uint32_t seq = 0,
//                                   bool do_crc = false)
//     {
//         const std::uint32_t total = static_cast<std::uint32_t>(16 + size);
//         std::vector<char> out(total);

//         write_le32(out.data()+0,  total);
//         write_le16(out.data()+4,  cmd);
//         out[6] = static_cast<char>(flags);             // [EXTENSION POINT]
//         out[7] = static_cast<char>(proto_ver);         // [EXTENSION POINT]
//         write_le32(out.data()+8,  seq);                // [EXTENSION POINT]
//         write_le32(out.data()+12, do_crc ? crc32_stub(static_cast<const char*>(data), size) : 0); // [EXTENSION POINT]

//         if (size && data) std::memcpy(out.data()+16, data, size);
//         return out;
//     }

//     // Parse header only (payload는 바깥에서 슬라이스)
//     // [EXTENSION POINT] 여기서 flags/seq/crc 검증 로직을 단계적으로 추가할 수 있습니다.
//     static frame_header parse_header(const char* buf, std::size_t buf_sz) {
//         if (buf_sz < 16) throw std::runtime_error("short header");
//         frame_header h{};
//         h.len   = read_le32(buf+0);
//         h.cmd   = read_le16(buf+4);
//         h.flags = static_cast<std::uint8_t>(buf[6]);
//         h.ver   = static_cast<std::uint8_t>(buf[7]);
//         h.seq   = read_le32(buf+8);
//         h.crc32 = read_le32(buf+12);
//         return h;
//     }

// private:
//     // ---- Little-Endian helpers ----
//     static void write_le32(char* p, std::uint32_t v) {
//         p[0]=char(v & 0xFF); p[1]=char((v>>8)&0xFF); p[2]=char((v>>16)&0xFF); p[3]=char((v>>24)&0xFF);
//     }
//     static void write_le16(char* p, std::uint16_t v) {
//         p[0]=char(v & 0xFF); p[1]=char((v>>8)&0xFF);
//     }
//     static std::uint32_t read_le32(const char* p) {
//         return (std::uint8_t)p[0]
//              | (std::uint32_t(std::uint8_t(p[1]))<<8)
//              | (std::uint32_t(std::uint8_t(p[2]))<<16)
//              | (std::uint32_t(std::uint8_t(p[3]))<<24);
//     }
//     static std::uint16_t read_le16(const char* p) {
//         return (std::uint8_t)p[0] | (std::uint16_t)(std::uint8_t)(p[1]<<8);
//     }

//     // CRC 자리채우기 (현재 미사용)
//     // [EXTENSION POINT] 나중에 실제 CRC32 구현으로 교체하세요.
//     static std::uint32_t crc32_stub(const char*, std::size_t) { return 0; }
// };

// } // namespace lemondory::network




// #pragma once
// #include <cstdint>
// #include <vector>
// #include <functional>
// #include <cstring>

// namespace lemondory::network {

// using frame_callback = std::function<void(std::uint16_t msg_id,
//                                           const char* data, std::size_t size)>;

// class frame_codec {
// public:
//     explicit frame_codec(frame_callback cb) : cb_(std::move(cb)) {}

//     // 수신한 바이트 누적 → 완성 프레임 있으면 콜백 호출
//     void on_bytes(const char* data, std::size_t size) {
//         buffer_.insert(buffer_.end(), data, data + size);
//         parse_loop();
//     }

//     // 전송 프레임 생성: [len(4,LE) | msg_id(2) | payload]
//     static std::vector<char> pack(std::uint16_t msg_id,
//                                   const char* payload, std::size_t size) {
//         const std::uint32_t len = static_cast<std::uint32_t>(2 + size);
//         std::vector<char> out;
//         out.resize(4 + 2 + size);
//         // len LE
//         out[0] = static_cast<char>(len & 0xFF);
//         out[1] = static_cast<char>((len >> 8) & 0xFF);
//         out[2] = static_cast<char>((len >> 16) & 0xFF);
//         out[3] = static_cast<char>((len >> 24) & 0xFF);
//         // msg_id LE
//         out[4] = static_cast<char>(msg_id & 0xFF);
//         out[5] = static_cast<char>((msg_id >> 8) & 0xFF);
//         if (size) std::memcpy(out.data() + 6, payload, size);
//         return out;
//     }

// private:
//     void parse_loop() {
//         // 최소 헤더 6바이트
//         while (buffer_.size() >= 6) {
//             std::uint32_t len =  (static_cast<std::uint8_t>(buffer_[0])      )
//                                | (static_cast<std::uint8_t>(buffer_[1]) << 8 )
//                                | (static_cast<std::uint8_t>(buffer_[2]) << 16)
//                                | (static_cast<std::uint8_t>(buffer_[3]) << 24);
//             if (len > max_frame_) { // 프로토콜 방어
//                 // 비정상 패킷 → 버퍼 폐기
//                 buffer_.clear();
//                 return;
//             }
//             const std::size_t total = 4 + len; // 4바이트 len + 본문(len=msg_id+payload)
//             if (buffer_.size() < total) return; // 덜 옴

//             std::uint16_t msg_id = (static_cast<std::uint8_t>(buffer_[4]))
//                                   |(static_cast<std::uint8_t>(buffer_[5]) << 8);
//             const char* payload = buffer_.data() + 6;
//             const std::size_t payload_sz = len - 2;

//             if (cb_) cb_(msg_id, payload, payload_sz);

//             buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<std::ptrdiff_t>(total));
//         }
//     }

// private:
//     std::vector<char> buffer_;
//     frame_callback cb_;
//     static constexpr std::uint32_t max_frame_ = 1u << 20; // 1MB 상한
// };

// } // namespace lemondory::network

