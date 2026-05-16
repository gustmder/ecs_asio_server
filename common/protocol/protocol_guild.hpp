#pragma once
#include "protocol_base.hpp"

namespace lemondory::shared {

// ==================== 길드 생성 메시지 ====================

struct guild_create_req {
    std::string guild_name;
    std::string guild_description;
    std::uint32_t max_members;
    std::uint8_t guild_type; // 0: 일반, 1: PvP, 2: PvE, 3: 레이드
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_lp_string(out, guild_name);
        write_lp_string(out, guild_description);
        write_le32(out, max_members);
        out.push_back(guild_type);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_create_req& out) {
        return read_lp_string(p, end, out.guild_name) &&
               read_lp_string(p, end, out.guild_description) &&
               (p + 4 <= end) && (out.max_members = read_le32(p), p += 4, true) &&
               (p < end) && (out.guild_type = *p++, true);
    }
};

struct guild_create_res {
    bool success;
    std::string message;
    std::uint32_t guild_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, guild_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_create_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message) &&
               (p + 4 <= end) && (out.guild_id = read_le32(p), p += 4, true);
    }
};

// ==================== 길드 가입 메시지 ====================

struct guild_join_req {
    std::uint32_t guild_id;
    std::string application_message;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        write_lp_string(out, application_message);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_join_req& out) {
        return (p + 4 <= end) && (out.guild_id = read_le32(p), p += 4, true) &&
               read_lp_string(p, end, out.application_message);
    }
};

struct guild_join_res {
    bool success;
    std::string message;
    std::uint32_t application_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, application_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_join_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message) &&
               (p + 4 <= end) && (out.application_id = read_le32(p), p += 4, true);
    }
};

// ==================== 길드 탈퇴 메시지 ====================

struct guild_leave_req {
    std::uint32_t guild_id;
    std::string reason;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        write_lp_string(out, reason);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_leave_req& out) {
        return (p + 4 <= end) && (out.guild_id = read_le32(p), p += 4, true) &&
               read_lp_string(p, end, out.reason);
    }
};

struct guild_leave_res {
    bool success;
    std::string message;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_leave_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message);
    }
};

// ==================== 길드 정보 메시지 ====================

struct guild_info_req {
    std::uint32_t guild_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_info_req& out) {
        if (p + 4 > end) return false;
        out.guild_id = read_le32(p); p += 4;
        return true;
    }
};

struct guild_info_res {
    std::uint32_t guild_id;
    std::string guild_name;
    std::string guild_description;
    std::uint32_t member_count;
    std::uint32_t max_members;
    std::uint8_t guild_type;
    std::uint32_t guild_level;
    std::uint32_t guild_exp;
    std::uint32_t leader_id;
    std::string leader_name;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        write_lp_string(out, guild_name);
        write_lp_string(out, guild_description);
        write_le32(out, member_count);
        write_le32(out, max_members);
        out.push_back(guild_type);
        write_le32(out, guild_level);
        write_le32(out, guild_exp);
        write_le32(out, leader_id);
        write_lp_string(out, leader_name);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_info_res& out) {
        if (p + 24 > end) return false;
        out.guild_id = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.guild_name)) return false;
        if (!read_lp_string(p, end, out.guild_description)) return false;
        out.member_count = read_le32(p); p += 4;
        out.max_members = read_le32(p); p += 4;
        out.guild_type = *p++; p += 1;
        out.guild_level = read_le32(p); p += 4;
        out.guild_exp = read_le32(p); p += 4;
        out.leader_id = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.leader_name)) return false;
        return true;
    }
};

// ==================== 길드 멤버 관리 메시지 ====================

struct guild_member_list_req {
    std::uint32_t guild_id;
    std::uint32_t page; // 페이지 번호 (0부터 시작)
    std::uint32_t page_size; // 페이지당 멤버 수
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        write_le32(out, page);
        write_le32(out, page_size);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_member_list_req& out) {
        if (p + 12 > end) return false;
        out.guild_id = read_le32(p); p += 4;
        out.page = read_le32(p); p += 4;
        out.page_size = read_le32(p); p += 4;
        return true;
    }
};

struct guild_member_info {
    std::uint32_t player_id;
    std::string player_name;
    std::uint32_t level;
    std::uint8_t rank; // 0: 일반멤버, 1: 부길드장, 2: 길드장
    std::uint32_t join_date;
    std::uint32_t last_online;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, player_id);
        write_lp_string(out, player_name);
        write_le32(out, level);
        out.push_back(rank);
        write_le32(out, join_date);
        write_le32(out, last_online);
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_member_info& out) {
        if (p + 16 > end) return false;
        out.player_id = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.player_name)) return false;
        out.level = read_le32(p); p += 4;
        out.rank = *p++; p += 1;
        out.join_date = read_le32(p); p += 4;
        out.last_online = read_le32(p); p += 4;
        return true;
    }
};

struct guild_member_list_res {
    std::uint32_t guild_id;
    std::uint32_t total_members;
    std::uint32_t page;
    std::uint32_t page_size;
    std::vector<guild_member_info> members;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, guild_id);
        write_le32(out, total_members);
        write_le32(out, page);
        write_le32(out, page_size);
        write_le32(out, static_cast<std::uint32_t>(members.size()));
        for (const auto& member : members) {
            auto member_data = member.serialize();
            out.insert(out.end(), member_data.begin(), member_data.end());
        }
        return out;
    }
    
    static bool parse(const char* p, const char* end, guild_member_list_res& out) {
        if (p + 16 > end) return false;
        out.guild_id = read_le32(p); p += 4;
        out.total_members = read_le32(p); p += 4;
        out.page = read_le32(p); p += 4;
        out.page_size = read_le32(p); p += 4;
        
        std::uint32_t member_count = read_le32(p); p += 4;
        out.members.clear();
        out.members.reserve(member_count);
        
        for (std::uint32_t i = 0; i < member_count; ++i) {
            guild_member_info member;
            if (!guild_member_info::parse(p, end, member)) return false;
            out.members.push_back(member);
        }
        return true;
    }
};

} // namespace lemondory::shared

