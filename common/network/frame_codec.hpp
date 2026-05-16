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

    // 오프셋 기반 파서 — erase 없이 읽기 위치만 전진 (O(1))
    // acc_offset: 현재 읽기 위치, 파싱 성공 시 소비한 만큼 전진
    // crc_ok: payload CRC 일치 여부 (false 시 caller가 drop 처리)
    inline bool try_parse_one(const std::vector<char>& acc, std::size_t& acc_offset,
                              header_v1& out_hdr, std::string_view& out_payload,
                              bool& crc_ok)
    {
        const std::size_t avail = acc.size() - acc_offset;
        if (avail < 4) return false;

        const char* base = acc.data() + acc_offset;
        const std::uint32_t len_after_len = get_le32(base);
        if (len_after_len > k_max_frame_bytes) {
            acc_offset = acc.size(); // desync → skip all
            return false;
        }

        const std::size_t total = 4u + static_cast<std::size_t>(len_after_len);
        if (avail < total) return false;
        if (len_after_len < k_header_after_len) {
            acc_offset = acc.size();
            return false;
        }

        out_hdr.cmd   = get_le16(base + 4);
        out_hdr.flags = static_cast<std::uint8_t>(base[6]);
        out_hdr.ver   = static_cast<std::uint8_t>(base[7]);
        out_hdr.seq   = get_le32(base + 8);
        out_hdr.crc   = get_le32(base + 12);

        const char* payload_ptr = base + 16;
        const std::size_t payload_sz = static_cast<std::size_t>(len_after_len) - k_header_after_len;
        out_payload = std::string_view(payload_ptr, payload_sz);

        crc_ok = (crc32(payload_ptr, payload_sz) == out_hdr.crc);

        acc_offset += total;
        return true;
    }

} // namespace lemondory::network::frame_codec
