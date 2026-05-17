#include <cstdint>
#include "server/game/game_server.hpp"
#include "common/log.hpp"

using namespace lemondory::core;
using namespace lemondory::network;
using namespace lemondory::game;

#if defined(__APPLE__)
#include <signal.h>
static int _ignore_sigpipe = [](){ ::signal(SIGPIPE, SIG_IGN); return 0; }();
#endif

int main() {
    log_init();

    LOGI("Starting Game Server...");

    asio::io_context io;

    lemondory::network::socket_option opt;
    opt.set_reuse_address(true).set_backlog(128).set_tcp_no_delay(true);

    lemondory::game::game_server server;
    if (!server.init(io, opt, "0.0.0.0", 12345)) {
        LOGE("Server init/listen failed");
        return 1;
    }

    LOGI("Server running on port 12345. Ctrl+C to exit.");
    io.run();

    return 0;
}

