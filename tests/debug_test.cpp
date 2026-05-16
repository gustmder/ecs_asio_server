// debug_test.cpp - 디버깅 연습용 간단한 프로그램
#include <iostream>
#include <vector>
#include <string>
#include "../core/packet_buffer.hpp"

using namespace lemondory::core;

void test_packet_buffer_debug() {
    std::cout << "=== Packet Buffer Debug Test ===" << std::endl;
    
    // 1. 동적 버퍼 생성
    packet_buffer buffer(1024, data_option::none);
    std::cout << "Created dynamic buffer, capacity: " << buffer.capacity() << std::endl;
    
    // 2. 데이터 추가
    std::string test_data = "Hello, Debug World!";
    buffer.append(test_data);
    std::cout << "Added data: " << test_data << std::endl;
    std::cout << "Current offset: " << buffer.offset() << std::endl;
    std::cout << "Remaining size: " << buffer.remaining_size() << std::endl;
    
    // 3. 오프셋 이동 (여기서 브레이크포인트 설정)
    buffer.set_offset(5);
    std::cout << "Moved offset to: " << buffer.offset() << std::endl;
    
    // 4. 추가 데이터
    buffer.append("Debug", 5);
    std::cout << "Added more data" << std::endl;
    std::cout << "Final offset: " << buffer.offset() << std::endl;
    
    // 5. 버퍼 내용 출력
    std::string content(buffer.raw(), buffer.offset());
    std::cout << "Buffer content: " << content << std::endl;
}

void test_network_debug() {
    std::cout << "\n=== Network Debug Test ===" << std::endl;
    
    // 네트워크 관련 변수들
    int channel_id = 1;
    std::string player_name = "DebugPlayer";
    bool is_connected = true;
    
    std::cout << "Channel ID: " << channel_id << std::endl;
    std::cout << "Player: " << player_name << std::endl;
    std::cout << "Connected: " << (is_connected ? "Yes" : "No") << std::endl;
    
    // 메시지 처리 시뮬레이션
    std::vector<std::string> messages = {
        "PING",
        "LOGIN",
        "MOVE",
        "CHAT"
    };
    
    for (const auto& msg : messages) {
        std::cout << "Processing message: " << msg << std::endl;
        // 여기서 브레이크포인트 설정하여 메시지 처리 과정 확인
    }
}

int main() {
    std::cout << "Starting Debug Test..." << std::endl;
    
    try {
        test_packet_buffer_debug();
        test_network_debug();
        
        std::cout << "\n=== Debug Test Completed Successfully! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}


