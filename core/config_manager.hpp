#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace lemondory::core {

// JSON 파싱을 위한 간단한 설정 관리자
class ConfigManager {
public:
    static ConfigManager& instance();
    
    // 설정 파일 로드
    bool load_config(const std::string& config_file);
    
    // 설정 값 조회
    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_int(const std::string& key, int default_value = 0) const;
    float get_float(const std::string& key, float default_value = 0.0f) const;
    bool get_bool(const std::string& key, bool default_value = false) const;
    
    // 서버 설정
    std::string get_server_name() const;
    int get_server_port() const;
    int get_max_connections() const;
    int get_tick_rate() const;
    
    // 스레딩 설정
    bool is_main_thread_enabled() const;
    std::vector<std::unordered_map<std::string, std::string>> get_map_configs() const;
    
    // ECS 설정
    bool is_ecs_enabled() const;
    std::vector<std::string> get_ecs_systems() const;
    
    // 로깅 설정
    std::string get_log_level() const;
    bool is_file_output_enabled() const;
    bool is_console_output_enabled() const;
    std::string get_log_file() const;
    
    // 게임 설정
    int get_max_players_per_map() const;
    bool is_combat_enabled() const;
    bool is_pvp_enabled() const;
    bool is_guild_system_enabled() const;
    bool is_friend_system_enabled() const;
    
private:
    ConfigManager() = default;
    
    // 설정 값 저장소
    std::unordered_map<std::string, std::string> config_values_;
    mutable std::mutex config_mutex_;
    
    // JSON 파싱 헬퍼
    void parse_json_simple(const std::string& json_content);
    std::string extract_value(const std::string& json, const std::string& key) const;
};

} // namespace lemondory::core
