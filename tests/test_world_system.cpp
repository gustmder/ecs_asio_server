#include <iostream>
#include <thread>
#include <chrono>
#include "game/world_system.hpp"

using namespace lemondory::game;

int main() {
    std::cout << "=== World System Test ===\n";
    
    // 월드 시스템 생성 및 초기화
    WorldSystem world;
    if (!world.initialize()) {
        std::cerr << "Failed to initialize world system\n";
        return 1;
    }
    
    std::cout << "World system initialized successfully!\n";
    
    // 맵 정보 출력
    auto maps = world.get_all_maps();
    std::cout << "Available maps:\n";
    for (const auto& map : maps) {
        std::cout << "  - Map " << map.map_id << ": " << map.map_name 
                  << " (max players: " << map.max_players << ")\n";
    }
    
    // 테스트 플레이어 추가
    std::cout << "\nAdding test players...\n";
    world.add_player(1, "Player1", 1, 0.0f, 0.0f, 0.0f);
    world.add_player(2, "Player2", 1, 5.0f, 5.0f, 0.0f);
    world.add_player(3, "Player3", 2, 10.0f, 10.0f, 0.0f);
    
    // 플레이어 위치 확인
    std::cout << "Player locations:\n";
    for (int i = 1; i <= 3; ++i) {
        int map_id = world.get_player_map(i);
        std::cout << "  Player " << i << " is in map " << map_id << "\n";
    }
    
    // 패킷 라우팅 테스트
    std::cout << "\nTesting packet routing...\n";
    std::vector<char> move_data = {0x00, 0x00, 0x20, 0x41, 0x00, 0x00, static_cast<char>(0xA0), 0x40, 0x00, 0x00, 0x00, 0x00}; // x=10, y=5, z=0
    world.route_packet(1, 1002, move_data); // MOVE 패킷
    
    std::vector<char> chat_data = {'H', 'e', 'l', 'l', 'o'};
    world.route_packet(2, 1003, chat_data); // CHAT 패킷
    
    // 잠시 대기 (맵 스레드가 패킷을 처리할 시간)
    std::cout << "Waiting for map threads to process packets...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 플레이어 이동 테스트
    std::cout << "\nTesting player movement between maps...\n";
    world.move_player(1, 2, 15.0f, 15.0f, 0.0f);
    std::cout << "Player 1 moved to map " << world.get_player_map(1) << "\n";
    
    // 플레이어 제거 테스트
    std::cout << "\nRemoving player 3...\n";
    world.remove_player(3);
    std::cout << "Player 3 location after removal: " << world.get_player_map(3) << "\n";
    
    std::cout << "\n=== World System Test Completed Successfully! ===\n";
    return 0;
}
