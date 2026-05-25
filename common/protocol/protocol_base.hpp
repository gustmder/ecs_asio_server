#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace lemondory::shared {

// ==================== 공통 유틸리티 ====================

// Little-Endian 인코딩/디코딩
inline void write_le16(std::vector<char>& out, std::uint16_t v) {
    out.push_back(char(v & 0xFF));
    out.push_back(char((v >> 8) & 0xFF));
}

inline void write_le32(std::vector<char>& out, std::uint32_t v) {
    out.push_back(char(v & 0xFF));
    out.push_back(char((v >> 8) & 0xFF));
    out.push_back(char((v >> 16) & 0xFF));
    out.push_back(char((v >> 24) & 0xFF));
}

inline std::uint16_t read_le16(const char* p) {
    return std::uint8_t(p[0]) | (std::uint16_t(std::uint8_t(p[1])) << 8);
}

inline std::uint32_t read_le32(const char* p) {
    return std::uint8_t(p[0]) | (std::uint32_t(std::uint8_t(p[1])) << 8)
         | (std::uint32_t(std::uint8_t(p[2])) << 16) | (std::uint32_t(std::uint8_t(p[3])) << 24);
}

// float ↔ LE-encoded uint32 (memcpy: no UB, no alignment assumption)
inline void write_float_le(std::vector<char>& out, float v) {
    std::uint32_t t; std::memcpy(&t, &v, sizeof(t)); write_le32(out, t);
}
inline float read_float_le(const char* p) {
    std::uint32_t t = read_le32(p); float v; std::memcpy(&v, &t, sizeof(v)); return v;
}

// length-prefixed string
inline void write_lp_string(std::vector<char>& out, const std::string& s) {
    write_le32(out, static_cast<std::uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

inline bool read_lp_string(const char*& p, const char* end, std::string& out) {
    if (end - p < 4) return false;
    auto len = read_le32(p); p += 4;
    if (len > static_cast<std::size_t>(end - p)) return false;
    out.assign(p, p + len); p += len;
    return true;
}

// ==================== 메시지 ID 정의 ====================

enum class MessageID : std::uint16_t {
    // 기본 메시지
    ECHO_REQ = 0x0001,
    ECHO_RES = 0x0002,
    
    // 로그인 관련
    LOGIN_REQ = 0x1001,
    LOGIN_RES = 0x1002,
    LOGOUT_REQ = 0x1003,
    LOGOUT_RES = 0x1004,
    
    // 플레이어 관련
    PLAYER_MOVE_REQ = 0x2001,
    PLAYER_MOVE_RES = 0x2002,
    PLAYER_INFO_REQ = 0x2003,
    PLAYER_INFO_RES = 0x2004,
    
    // 채팅 관련
    CHAT_REQ = 0x3001,
    CHAT_RES = 0x3002,
    CHAT_BROADCAST = 0x3003,
    
    // 전투 관련
    ATTACK_REQ = 0x4001,
    ATTACK_RES = 0x4002,
    DAMAGE_NOTIFY = 0x4003,
    
    // 맵 관련
    MAP_ENTER_REQ = 0x5001,
    MAP_ENTER_RES = 0x5002,
    MAP_LEAVE_REQ = 0x5003,
    MAP_LEAVE_RES = 0x5004,
    MAP_OBJECTS_UPDATE = 0x5005,
    
    // 길드 관련
    GUILD_CREATE_REQ = 0x6001,
    GUILD_CREATE_RES = 0x6002,
    GUILD_JOIN_REQ = 0x6003,
    GUILD_JOIN_RES = 0x6004,
    GUILD_LEAVE_REQ = 0x6005,
    GUILD_LEAVE_RES = 0x6006,
    GUILD_INFO_REQ = 0x6007,
    GUILD_INFO_RES = 0x6008
};

// ==================== 기본 메시지 ====================

struct echo_req {
    std::string text;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_lp_string(out, text);
        return out;
    }
    
    static bool parse(const char* p, const char* end, echo_req& out) {
        return read_lp_string(p, end, out.text);
    }
};

struct echo_res {
    std::string text;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_lp_string(out, text);
        return out;
    }
    
    static bool parse(const char* p, const char* end, echo_res& out) {
        return read_lp_string(p, end, out.text);
    }
};

// ==================== 로그인 메시지 ====================

struct login_req {
    std::string player_name;
    std::string session_token;  // Phase 1: "", Phase 3: JWT from auth server

    std::vector<char> serialize() const {
        std::vector<char> out;
        write_lp_string(out, player_name);
        write_lp_string(out, session_token);
        return out;
    }

    static bool parse(const char* p, const char* end, login_req& out) {
        return read_lp_string(p, end, out.player_name) &&
               read_lp_string(p, end, out.session_token);
    }
};

struct login_res {
    bool success;
    std::string message;
    std::uint32_t player_id;
    std::uint32_t map_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, player_id);
        write_le32(out, map_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, login_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message) &&
               (p + 8 <= end) && (out.player_id = read_le32(p), p += 4, out.map_id = read_le32(p), p += 4, true);
    }
};

} // namespace lemondory::shared
