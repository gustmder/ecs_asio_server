#include "client/game_bot.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const std::string host        = (argc > 1) ? argv[1] : "127.0.0.1";
    const int         port        = (argc > 2) ? std::stoi(argv[2]) : 12345;
    const std::string player_name = (argc > 3) ? argv[3] : "BotPlayer";

    lemondory::client::GameBot bot(player_name, host, port);

    if (!bot.connect()) {
        std::cerr << "접속 실패\n";
        return 1;
    }

    bot.run(10);
    bot.disconnect();

    return 0;
}
