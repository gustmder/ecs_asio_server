#include "config_manager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

namespace lemondory::core {

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load_config(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_file << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    parse_json_simple(json_content);
    
    std::cout << "Config loaded from: " << config_file << std::endl;
    return true;
}

std::string ConfigManager::get_string(const std::string& key, const std::string& default_value) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    auto it = config_values_.find(key);
    return it != config_values_.end() ? it->second : default_value;
}

int ConfigManager::get_int(const std::string& key, int default_value) const {
    std::string value = get_string(key);
    if (value.empty()) return default_value;
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

float ConfigManager::get_float(const std::string& key, float default_value) const {
    std::string value = get_string(key);
    if (value.empty()) return default_value;
    
    try {
        return std::stof(value);
    } catch (...) {
        return default_value;
    }
}

bool ConfigManager::get_bool(const std::string& key, bool default_value) const {
    std::string value = get_string(key);
    if (value.empty()) return default_value;
    
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value == "true" || value == "1";
}

// 서버 설정
std::string ConfigManager::get_server_name() const {
    return get_string("server.name", "Lemondory Game Server");
}

int ConfigManager::get_server_port() const {
    return get_int("server.port", 12345);
}

int ConfigManager::get_max_connections() const {
    return get_int("server.max_connections", 1000);
}

int ConfigManager::get_tick_rate() const {
    return get_int("server.tick_rate", 60);
}

// 스레딩 설정
bool ConfigManager::is_main_thread_enabled() const {
    return get_bool("threading.main_thread_enabled", true);
}

std::vector<std::unordered_map<std::string, std::string>> ConfigManager::get_map_configs() const {
    // 간단한 맵 설정 반환 (실제로는 JSON 배열 파싱 필요)
    std::vector<std::unordered_map<std::string, std::string>> maps;
    
    // 기본 맵들
    std::unordered_map<std::string, std::string> map1;
    map1["id"] = "1";
    map1["name"] = "Forest";
    map1["width"] = "1000.0";
    map1["height"] = "1000.0";
    map1["max_players"] = "100";
    maps.push_back(map1);
    
    std::unordered_map<std::string, std::string> map2;
    map2["id"] = "2";
    map2["name"] = "Dungeon";
    map2["width"] = "800.0";
    map2["height"] = "800.0";
    map2["max_players"] = "50";
    maps.push_back(map2);
    
    return maps;
}

// ECS 설정
bool ConfigManager::is_ecs_enabled() const {
    return get_bool("ecs.enabled", true);
}

std::vector<std::string> ConfigManager::get_ecs_systems() const {
    // 기본 시스템들
    return {"MovementSystem", "AISystem", "CombatSystem", "MapSystem"};
}

// 로깅 설정
std::string ConfigManager::get_log_level() const {
    return get_string("logging.level", "info");
}

bool ConfigManager::is_file_output_enabled() const {
    return get_bool("logging.file_output", true);
}

bool ConfigManager::is_console_output_enabled() const {
    return get_bool("logging.console_output", true);
}

std::string ConfigManager::get_log_file() const {
    return get_string("logging.log_file", "logs/game_server.log");
}

// 게임 설정
int ConfigManager::get_max_players_per_map() const {
    return get_int("game.max_players_per_map", 100);
}

bool ConfigManager::is_combat_enabled() const {
    return get_bool("game.combat_enabled", true);
}

bool ConfigManager::is_pvp_enabled() const {
    return get_bool("game.pvp_enabled", true);
}

bool ConfigManager::is_guild_system_enabled() const {
    return get_bool("game.guild_system", true);
}

bool ConfigManager::is_friend_system_enabled() const {
    return get_bool("game.friend_system", true);
}

void ConfigManager::parse_json_simple(const std::string& json_content) {
    // 간단한 JSON 파싱 (실제로는 nlohmann/json 등 사용 권장)
    // 여기서는 기본값들만 설정
    
    config_values_["server.name"] = "Lemondory Game Server";
    config_values_["server.port"] = "12345";
    config_values_["server.max_connections"] = "1000";
    config_values_["server.tick_rate"] = "60";
    
    config_values_["threading.main_thread_enabled"] = "true";
    
    config_values_["ecs.enabled"] = "true";
    
    config_values_["logging.level"] = "info";
    config_values_["logging.file_output"] = "true";
    config_values_["logging.console_output"] = "true";
    config_values_["logging.log_file"] = "logs/game_server.log";
    
    config_values_["game.max_players_per_map"] = "100";
    config_values_["game.combat_enabled"] = "true";
    config_values_["game.pvp_enabled"] = "true";
    config_values_["game.guild_system"] = "true";
    config_values_["game.friend_system"] = "true";
}

std::string ConfigManager::extract_value(const std::string& json, const std::string& key) const {
    // 간단한 JSON 값 추출 (실제로는 더 정교한 파싱 필요)
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    pos = json.find("\"", pos);
    if (pos == std::string::npos) return "";
    
    size_t start = pos + 1;
    size_t end = json.find("\"", start);
    if (end == std::string::npos) return "";
    
    return json.substr(start, end - start);
}

} // namespace lemondory::core
