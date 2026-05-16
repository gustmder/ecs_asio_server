#pragma once
#include "protocol_base.hpp"

namespace lemondory::shared {

// ==================== 채팅 메시지 ====================

struct chat_req {
    std::string message;
    std::uint8_t channel; // 0: 전체, 1: 길드, 2: 파티, 3: 귓속말
    std::uint32_t target_id; // 귓속말 대상 ID (channel이 3일 때만 사용)
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_lp_string(out, message);
        out.push_back(channel);
        write_le32(out, target_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_req& out) {
        return read_lp_string(p, end, out.message) &&
               (p < end) && (out.channel = *p++, true) &&
               (p + 4 <= end) && (out.target_id = read_le32(p), p += 4, true);
    }
};

struct chat_res {
    bool success;
    std::string message;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message);
    }
};

struct chat_broadcast {
    std::uint32_t sender_id;
    std::string sender_name;
    std::string message;
    std::uint8_t channel;
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, sender_id);
        write_lp_string(out, sender_name);
        write_lp_string(out, message);
        out.push_back(channel);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_broadcast& out) {
        return (p + 4 <= end) && (out.sender_id = read_le32(p), p += 4, true) &&
               read_lp_string(p, end, out.sender_name) &&
               read_lp_string(p, end, out.message) &&
               (p < end) && (out.channel = *p++, true) &&
               (p + 4 <= end) && (out.timestamp = read_le32(p), p += 4, true);
    }
};

// ==================== 채팅 시스템 메시지 ====================

struct chat_system_message {
    std::uint32_t message_type; // 0: 시스템, 1: 공지, 2: 이벤트
    std::string title;
    std::string content;
    std::uint32_t priority; // 0: 일반, 1: 중요, 2: 긴급
    std::uint32_t timestamp;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        write_le32(out, message_type);
        write_lp_string(out, title);
        write_lp_string(out, content);
        write_le32(out, priority);
        write_le32(out, timestamp);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_system_message& out) {
        if (p + 12 > end) return false;
        out.message_type = read_le32(p); p += 4;
        if (!read_lp_string(p, end, out.title)) return false;
        if (!read_lp_string(p, end, out.content)) return false;
        out.priority = read_le32(p); p += 4;
        out.timestamp = read_le32(p); p += 4;
        return true;
    }
};

// ==================== 채팅 필터 메시지 ====================

struct chat_filter_req {
    std::uint8_t filter_type; // 0: 스팸, 1: 욕설, 2: 광고
    std::string target_message;
    std::uint32_t reporter_id;
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(filter_type);
        write_lp_string(out, target_message);
        write_le32(out, reporter_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_filter_req& out) {
        return (p < end) && (out.filter_type = *p++, true) &&
               read_lp_string(p, end, out.target_message) &&
               (p + 4 <= end) && (out.reporter_id = read_le32(p), p += 4, true);
    }
};

struct chat_filter_res {
    bool success;
    std::string message;
    std::uint32_t case_id; // 신고 케이스 ID
    
    std::vector<char> serialize() const {
        std::vector<char> out;
        out.push_back(success ? 1 : 0);
        write_lp_string(out, message);
        write_le32(out, case_id);
        return out;
    }
    
    static bool parse(const char* p, const char* end, chat_filter_res& out) {
        return (p < end) && (out.success = (*p++ != 0), true) &&
               read_lp_string(p, end, out.message) &&
               (p + 4 <= end) && (out.case_id = read_le32(p), p += 4, true);
    }
};

} // namespace lemondory::shared
