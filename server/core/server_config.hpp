#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace lemondory::core {

struct MapConfig {
    int         id          = 0;
    std::string name        = "";
    float       width       = 1000.0f;
    float       height      = 1000.0f;
    int         max_players = 100;
};

struct ServerConfig {
    // ── 네트워크 ─────────────────────────────────────────────────
    std::string host            = "0.0.0.0";
    uint16_t    port            = 12345;
    int         backlog         = 128;
    int         max_connections = 1000;
    bool        tcp_no_delay    = true;

    // ── 로깅 ─────────────────────────────────────────────────────
    std::string log_level       = "debug";   // trace/debug/info/warn/error/critical
    bool        log_file        = false;
    std::string log_file_path   = "logs/server.log";

    // ── 스레딩 ───────────────────────────────────────────────────
    int         io_threads          = 1;     // asio io_context 스레드 수 (현재 미사용, 확장용)
    bool        map_threads_enabled = true;

    // ── 게임 ─────────────────────────────────────────────────────
    int  tick_rate        = 60;
    bool use_ecs          = true;
    int  memory_pool_size = 1000;

    // ── 맵 목록 ──────────────────────────────────────────────────
    std::vector<MapConfig> maps = {
        {1, "Forest",  1000.0f, 1000.0f, 100},
        {2, "Dungeon",  800.0f,  800.0f,  50},
        {3, "City",   1500.0f, 1500.0f, 200},
    };

    // config/server_config.json 파일이 있으면 덮어쓰고,
    // 없거나 파싱 실패하면 위 기본값 그대로 사용
    static ServerConfig load(const std::string& path = "config/server_config.json");
};

} // namespace lemondory::core
