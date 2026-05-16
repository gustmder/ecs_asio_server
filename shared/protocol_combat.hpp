#pragma once
#include "protocol_base.hpp"

namespace lemondory::shared {

// ==================== 전투 메시지 ====================

struct attack_req {
    std::uint32_t target_id;
    std::uint32_t skill_id;
    float x, y, z; // 공격 위치
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, target_id);
        write_le32(out, skill_id);
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&x));
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&y));
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&z));
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, attack_req& out) {
        if (p + 24 > end) return false;
        out.target_id = read_le32(p); p += 4;
        out.skill_id = read_le32(p); p += 4;
        out.x = *reinterpret_cast<const float*>(p); p += 4;
        out.y = *reinterpret_cast<const float*>(p); p += 4;
        out.z = *reinterpret_cast<const float*>(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

struct attack_res {
    bool success;
    std::string message;
    std::uint32_t attack_id;
    std::uint32_t damage;
    std::uint32_t target_remaining_hp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, attack_id);
        write_le32(out, damage);
        write_le32(out, target_remaining_hp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, attack_res& out) {
        if (p + 12 > end) return false;
        out.success = (*p++ != 0);
        if (!read_lp_string(p, end, out.message)) return false;
        out.attack_id = read_le32(p); p += 4;
        out.damage = read_le32(p); p += 4;
        out.target_remaining_hp = read_le32(p); p += 4;
        return true;
    }
};

struct damage_notify {
    std::uint32_t attacker_id;
    std::uint32_t target_id;
    std::uint32_t damage;
    std::uint32_t remaining_hp;
    std::uint32_t attack_id;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, attacker_id);
        write_le32(out, target_id);
        write_le32(out, damage);
        write_le32(out, remaining_hp);
        write_le32(out, attack_id);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, damage_notify& out) {
        if (p + 24 > end) return false;
        out.attacker_id = read_le32(p); p += 4;
        out.target_id = read_le32(p); p += 4;
        out.damage = read_le32(p); p += 4;
        out.remaining_hp = read_le32(p); p += 4;
        out.attack_id = read_le32(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 스킬 메시지 ====================

struct skill_use_req {
    std::uint32_t skill_id;
    std::uint32_t target_id;
    float x, y, z; // 스킬 사용 위치
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, skill_id);
        write_le32(out, target_id);
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&x));
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&y));
        write_le32(out, *reinterpret_cast<const std::uint32_t*>(&z));
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, skill_use_req& out) {
        if (p + 24 > end) return false;
        out.skill_id = read_le32(p); p += 4;
        out.target_id = read_le32(p); p += 4;
        out.x = *reinterpret_cast<const float*>(p); p += 4;
        out.y = *reinterpret_cast<const float*>(p); p += 4;
        out.z = *reinterpret_cast<const float*>(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

struct skill_use_res {
    bool success;
    std::string message;
    std::uint32_t skill_id;
    std::uint32_t cooldown_time;
    std::uint32_t mana_cost;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, skill_id);
        write_le32(out, cooldown_time);
        write_le32(out, mana_cost);
        return out;
    }
    
    static bool parse(const char* p, const char* end, skill_use_res& out) {
        if (p + 12 > end) return false;
        out.success = (*p++ != 0);
        if (!read_lp_string(p, end, out.message)) return false;
        out.skill_id = read_le32(p); p += 4;
        out.cooldown_time = read_le32(p); p += 4;
        out.mana_cost = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 상태 효과 메시지 ====================

struct status_effect_notify {
    std::uint32_t target_id;
    std::uint32_t effect_id;
    std::uint8_t effect_type; // 0: 버프, 1: 디버프, 2: 도트, 3: 핫
    std::uint32_t duration;
    std::uint32_t value;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, target_id);
        write_le32(out, effect_id);
        out.push_back(effect_type);
        write_le32(out, duration);
        write_le32(out, value);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, status_effect_notify& out) {
        if (p + 20 > end) return false;
        out.target_id = read_le32(p); p += 4;
        out.effect_id = read_le32(p); p += 4;
        out.effect_type = *p++; p += 1;
        out.duration = read_le32(p); p += 4;
        out.value = read_le32(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 전투 결과 메시지 ====================

struct combat_result_notify {
    std::uint32_t winner_id;
    std::uint32_t loser_id;
    std::uint32_t experience_gained;
    std::uint32_t gold_gained;
    std::vector<std::uint32_t> item_ids;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, winner_id);
        write_le32(out, loser_id);
        write_le32(out, experience_gained);
        write_le32(out, gold_gained);
        write_le32(out, static_cast<std::uint32_t>(item_ids.size()));
        for (auto item_id : item_ids) {
            write_le32(out, item_id);
        }
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, combat_result_notify& out) {
        if (p + 20 > end) return false;
        out.winner_id = read_le32(p); p += 4;
        out.loser_id = read_le32(p); p += 4;
        out.experience_gained = read_le32(p); p += 4;
        out.gold_gained = read_le32(p); p += 4;
        
        std::uint32_t item_count = read_le32(p); p += 4;
        out.item_ids.clear();
        out.item_ids.reserve(item_count);
        
        for (std::uint32_t i = 0; i < item_count; ++i) {
            if (p + 4 > end) return false;
            out.item_ids.push_back(read_le32(p)); p += 4;
        }
        
        if (p + 4 > end) return false;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

} // namespace lemondory::shared

