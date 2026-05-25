#pragma once
#include "protocol_base.hpp"

namespace lemondory::shared {

// ==================== 맵 진입 메시지 ====================

struct map_enter_req {
    std::uint32_t map_id;
    float x, y, z; // 진입 위치
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, map_id);
        write_float_le(out, x);
        write_float_le(out, y);
        write_float_le(out, z);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_enter_req& out) {
        if (p + 20 > end) return false;
        out.map_id    = read_le32(p);    p += 4;
        out.x         = read_float_le(p); p += 4;
        out.y         = read_float_le(p); p += 4;
        out.z         = read_float_le(p); p += 4;
        out.timestamp = read_le32(p);    p += 4;
        return true;
    }
};

struct map_enter_res {
    bool success;
    std::string message;
    std::uint32_t map_id;
    std::string map_name;
    float x, y, z;
    std::uint32_t player_count;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, map_id);
        write_lp_string(out, map_name);
        write_float_le(out, x);
        write_float_le(out, y);
        write_float_le(out, z);
        write_le32(out, player_count);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_enter_res& out) {
        if (p + 20 > end) return false;
        out.success = (*p++ != 0);
        if (!read_lp_string(p, end, out.message)) return false;
        out.map_id = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.map_name)) return false;
        out.x            = read_float_le(p); p += 4;
        out.y            = read_float_le(p); p += 4;
        out.z            = read_float_le(p); p += 4;
        out.player_count = read_le32(p);    p += 4;
        return true;
    }
};

// ==================== 맵 퇴장 메시지 ====================

struct map_leave_req {
    std::uint32_t map_id;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, map_id);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_leave_req& out) {
        if (p + 8 > end) return false;
        out.map_id = read_le32(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

struct map_leave_res {
    bool success;
    std::string message;
    std::uint32_t map_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, map_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_leave_res& out) {
        if (p + 8 > end) return false;
        out.success = (*p++ != 0);
        if (!read_lp_string(p, end, out.message)) return false;
        out.map_id = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 맵 객체 정보 ====================

struct map_object_info {
    std::uint32_t object_id;
    std::uint8_t object_type; // 0: 플레이어, 1: 몬스터, 2: NPC, 3: 아이템
    std::string object_name;
    float x, y, z;
    float rotation;
    std::uint32_t level;
    std::uint32_t hp;
    std::uint32_t max_hp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, object_id);
        out.push_back(object_type);
        write_lp_string(out, object_name);
        write_float_le(out, x);
        write_float_le(out, y);
        write_float_le(out, z);
        write_float_le(out, rotation);
        write_le32(out, level);
        write_le32(out, hp);
        write_le32(out, max_hp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_object_info& out) {
        if (p + 28 > end) return false;
        out.object_id = read_le32(p); p += 4;
        out.object_type = static_cast<std::uint8_t>(*p++);
        if (!read_lp_string(p, end, out.object_name)) return false;
        out.x        = read_float_le(p); p += 4;
        out.y        = read_float_le(p); p += 4;
        out.z        = read_float_le(p); p += 4;
        out.rotation = read_float_le(p); p += 4;
        out.level = read_le32(p); p += 4;
        out.hp = read_le32(p); p += 4;
        out.max_hp = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 맵 객체 업데이트 메시지 ====================

struct map_objects_update {
    std::uint32_t map_id;
    std::uint32_t update_type; // 0: 전체, 1: 추가, 2: 제거, 3: 수정
    std::vector<map_object_info> objects;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, map_id);
        write_le32(out, update_type);
        write_le32(out, static_cast<std::uint32_t>(objects.size()));
        for (const auto& obj : objects) {
            auto obj_data = obj.serialize();
            out.insert(out.end(), obj_data.begin(), obj_data.end());
        }
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_objects_update& out) {
        if (p + 12 > end) return false;
        out.map_id = read_le32(p); p += 4;
        out.update_type = read_le32(p); p += 4;
        
        std::uint32_t object_count = read_le32(p); p += 4;
        out.objects.clear();
        out.objects.reserve(object_count);
        
        for (std::uint32_t i = 0; i < object_count; ++i) {
            map_object_info obj;
            if (!map_object_info::parse(p, end, obj)) return false;
            out.objects.push_back(obj);
        }
        
        if (p + 4 > end) return false;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 맵 정보 메시지 ====================

struct map_info_req {
    std::uint32_t map_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, map_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_info_req& out) {
        if (p + 4 > end) return false;
        out.map_id = read_le32(p); p += 4;
        return true;
    }
};

struct map_info_res {
    std::uint32_t map_id;
    std::string map_name;
    std::string map_description;
    std::uint32_t map_width;
    std::uint32_t map_height;
    std::uint32_t max_players;
    std::uint32_t current_players;
    std::uint8_t map_type; // 0: 일반, 1: 던전, 2: PvP, 3: 레이드
    std::uint32_t required_level;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, map_id);
        write_lp_string(out, map_name);
        write_lp_string(out, map_description);
        write_le32(out, map_width);
        write_le32(out, map_height);
        write_le32(out, max_players);
        write_le32(out, current_players);
        out.push_back(map_type);
        write_le32(out, required_level);
        return out;
    }
    
    static bool parse(const char* p, const char* end, map_info_res& out) {
        if (p + 24 > end) return false;
        out.map_id = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.map_name)) return false;
        if (!read_lp_string(p, end, out.map_description)) return false;
        out.map_width = read_le32(p); p += 4;
        out.map_height = read_le32(p); p += 4;
        out.max_players = read_le32(p); p += 4;
        out.current_players = read_le32(p); p += 4;
        out.map_type       = static_cast<std::uint8_t>(*p++);
        out.required_level = read_le32(p); p += 4;
        return true;
    }
};

} // namespace lemondory::shared

