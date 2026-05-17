#include "server_config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include "common/log.hpp"

namespace lemondory::core {

using json = nlohmann::json;

ServerConfig ServerConfig::load(const std::string& path) {
    ServerConfig cfg;

    std::ifstream f(path);
    if (!f.is_open()) {
        LOGI("Config file not found ({}), using defaults.", path);
        return cfg;
    }

    json j;
    try {
        j = json::parse(f);
    } catch (const std::exception& e) {
        LOGW("Config parse error ({}): {}. Using defaults.", path, e.what());
        return cfg;
    }

    // ── 네트워크 ─────────────────────────────────────────────────
    if (auto s = j.find("server"); s != j.end()) {
        if (s->contains("host"))            cfg.host            = (*s)["host"].get<std::string>();
        if (s->contains("port"))            cfg.port            = (*s)["port"].get<uint16_t>();
        if (s->contains("backlog"))         cfg.backlog         = (*s)["backlog"].get<int>();
        if (s->contains("max_connections")) cfg.max_connections = (*s)["max_connections"].get<int>();
        if (s->contains("tcp_no_delay"))    cfg.tcp_no_delay    = (*s)["tcp_no_delay"].get<bool>();
        if (s->contains("tick_rate"))       cfg.tick_rate       = (*s)["tick_rate"].get<int>();
    }

    // ── 로깅 ─────────────────────────────────────────────────────
    if (auto l = j.find("logging"); l != j.end()) {
        if (l->contains("level"))          cfg.log_level     = (*l)["level"].get<std::string>();
        if (l->contains("file"))           cfg.log_file      = (*l)["file"].get<bool>();
        if (l->contains("file_path"))      cfg.log_file_path = (*l)["file_path"].get<std::string>();
    }

    // ── 스레딩 ───────────────────────────────────────────────────
    if (auto t = j.find("threading"); t != j.end()) {
        if (t->contains("io_threads"))          cfg.io_threads          = (*t)["io_threads"].get<int>();
        if (t->contains("map_threads_enabled")) cfg.map_threads_enabled = (*t)["map_threads_enabled"].get<bool>();
    }

    // ── 게임 ─────────────────────────────────────────────────────
    if (auto g = j.find("game"); g != j.end()) {
        if (g->contains("use_ecs"))          cfg.use_ecs          = (*g)["use_ecs"].get<bool>();
        if (g->contains("memory_pool_size")) cfg.memory_pool_size = (*g)["memory_pool_size"].get<int>();
    }

    // ── 맵 목록 ──────────────────────────────────────────────────
    if (j.contains("maps") && j["maps"].is_array() && !j["maps"].empty()) {
        cfg.maps.clear();
        for (const auto& m : j["maps"]) {
            MapConfig mc;
            if (m.contains("id"))          mc.id          = m["id"].get<int>();
            if (m.contains("name"))        mc.name        = m["name"].get<std::string>();
            if (m.contains("width"))       mc.width       = m["width"].get<float>();
            if (m.contains("height"))      mc.height      = m["height"].get<float>();
            if (m.contains("max_players")) mc.max_players = m["max_players"].get<int>();
            cfg.maps.push_back(mc);
        }
    }

    LOGI("Config loaded from {}.", path);
    return cfg;
}

} // namespace lemondory::core
