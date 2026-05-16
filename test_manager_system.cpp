#include <iostream>
#include <thread>
#include <chrono>
#include "game/managers/game_manager.hpp"
#include "game/managers/map_manager.hpp"

using namespace lemondory::game;

int main() {
    std::cout << "=== Manager System Test ===\n";
    
    // 게임 매니저 초기화
    if (!GAME_MANAGER.initialize()) {
        std::cerr << "Failed to initialize Game Manager\n";
        return 1;
    }
    
    std::cout << "Game Manager initialized successfully!\n";
    
    // 맵 매니저 테스트
    std::cout << "\n=== Map Manager Test ===\n";
    auto& map_manager = MAP_MANAGER;
    auto maps = map_manager.get_all_maps();
    std::cout << "Available maps:\n";
    for (const auto& map : maps) {
        std::cout << "  - Map " << map.map_id << ": " << map.map_name 
                  << " (max players: " << map.max_players << ")\n";
    }
    
    // 플레이어 추가
    std::cout << "\nAdding players to maps...\n";
    map_manager.add_player_to_map(1, 1); // Player 1 to Map 1
    map_manager.add_player_to_map(2, 1); // Player 2 to Map 1
    map_manager.add_player_to_map(3, 2); // Player 3 to Map 2
    
    // 플레이어 위치 확인
    std::cout << "Player locations:\n";
    for (int i = 1; i <= 3; ++i) {
        int map_id = map_manager.get_player_map(i);
        std::cout << "  Player " << i << " is in map " << map_id << "\n";
    }
    
    // 패킷 라우팅 테스트
    std::cout << "\nTesting packet routing...\n";
    std::vector<char> move_data = {0x00, 0x00, 0x20, 0x41, 0x00, 0x00, static_cast<char>(0xA0), 0x40, 0x00, 0x00, 0x00, 0x00};
    map_manager.route_packet(1, 1002, move_data); // MOVE 패킷
    
    std::vector<char> chat_data = {'H', 'e', 'l', 'l', 'o'};
    map_manager.route_packet(2, 1003, chat_data); // CHAT 패킷
    
    // 플레이어 이동 테스트
    std::cout << "\nTesting player movement between maps...\n";
    map_manager.move_player_to_map(1, 2);
    std::cout << "Player 1 moved to map " << map_manager.get_player_map(1) << "\n";
    
    // 통계 출력
    std::cout << "\nManager Statistics:\n";
    std::cout << "  Total players: " << map_manager.get_total_players() << "\n";
    std::cout << "  Map 1 players: " << map_manager.get_map_player_count(1) << "\n";
    std::cout << "  Map 2 players: " << map_manager.get_map_player_count(2) << "\n";
    std::cout << "  Active maps: " << map_manager.get_active_maps().size() << "\n";
    
    // FPS 테스트
    std::cout << "\nTesting update loop...\n";
    auto start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < 10; ++i) {
        GAME_MANAGER.update(0.016f); // 60 FPS 시뮬레이션
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<float>(end_time - start_time).count();
    
    std::cout << "Update loop completed in " << duration << " seconds\n";
    std::cout << "Current FPS: " << GAME_MANAGER.get_fps() << "\n";
    
    // 매니저 시스템 종료
    std::cout << "\nShutting down Manager System...\n";
    GAME_MANAGER.shutdown();
    
    std::cout << "\n=== Manager System Test Completed Successfully! ===\n";
    return 0;
}
