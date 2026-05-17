#include <cstdint>
#include "server/game/game_server.hpp"
#include "server/core/server_config.hpp"
#include "common/log.hpp"

using namespace lemondory::core;
using namespace lemondory::network;
using namespace lemondory::game;

#if defined(__APPLE__)
#include <signal.h>
static int _ignore_sigpipe = [](){ ::signal(SIGPIPE, SIG_IGN); return 0; }();
#endif

static spdlog::level::level_enum parse_log_level(const std::string& s) {
    if (s == "trace")    return spdlog::level::trace;
    if (s == "debug")    return spdlog::level::debug;
    if (s == "info")     return spdlog::level::info;
    if (s == "warn")     return spdlog::level::warn;
    if (s == "error")    return spdlog::level::err;
    if (s == "critical") return spdlog::level::critical;
    return spdlog::level::debug;
}

int main() {
    auto cfg = ServerConfig::load("config/server_config.json");

    log_init(parse_log_level(cfg.log_level));
    LOGI("Starting Game Server (port={}, log={})", cfg.port, cfg.log_level);

    asio::io_context io;

    lemondory::network::socket_option opt;
    opt.set_reuse_address(true)
       .set_backlog(cfg.backlog)
       .set_tcp_no_delay(cfg.tcp_no_delay);

    lemondory::game::game_server server;
    server.set_config(cfg);

    if (!server.init(io, opt, cfg.host, cfg.port)) {
        LOGE("Server init/listen failed");
        return 1;
    }

    LOGI("Server running on {}:{}. Ctrl+C to exit.", cfg.host, cfg.port);
    io.run();

    return 0;
}
