// test_packet_buffer.cpp
#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include "../core/packet_buffer.hpp"

using namespace lemondory::core;

void test_dynamic_buffer() {
    std::cout << "[TEST] Dynamic buffer test...\n";
    
    // 동적 버퍼 생성
    packet_buffer buffer(1024, data_option::none);
    assert(buffer.is_dynamic());
    assert(buffer.is_owner());
    assert(buffer.capacity() >= 1024);
    
    // 데이터 추가
    std::string test_data = "Hello, World!";
    buffer.append(test_data);
    assert(buffer.offset() == test_data.size());
    assert(buffer.remaining_size() == buffer.capacity() - test_data.size());
    
    // 오프셋 이동
    buffer.set_offset(5);
    assert(buffer.offset() == 5);
    
    // 추가 데이터
    buffer.append("Test", 4);
    assert(buffer.offset() == 9);
    
    std::cout << "[TEST] Dynamic buffer test PASSED\n";
}

void test_external_buffer() {
    std::cout << "[TEST] External buffer test...\n";
    
    // 외부 버퍼 생성
    char external_data[256] = {0};
    packet_buffer buffer(external_data, 0, 256, data_option::none);
    assert(!buffer.is_dynamic());
    assert(!buffer.is_owner());
    assert(buffer.capacity() == 256);
    
    // 데이터 추가
    buffer.append("External", 8);
    assert(buffer.offset() == 8);
    assert(std::memcmp(external_data, "External", 8) == 0);
    
    std::cout << "[TEST] External buffer test PASSED\n";
}

void test_move_semantics() {
    std::cout << "[TEST] Move semantics test...\n";
    
    packet_buffer buffer1(512, data_option::none);
    buffer1.append("Move test", 9);
    
    packet_buffer buffer2 = std::move(buffer1);
    assert(buffer2.is_dynamic());
    assert(buffer2.offset() == 9);
    assert(!buffer1.is_dynamic()); // 이동된 객체는 비어있음
    
    std::cout << "[TEST] Move semantics test PASSED\n";
}

void test_capacity_expansion() {
    std::cout << "[TEST] Capacity expansion test...\n";
    
    packet_buffer buffer(100, data_option::none);
    assert(buffer.capacity() >= 100);
    
    // 큰 데이터 추가 (용량 확장)
    std::string large_data(200, 'A');
    buffer.append(large_data);
    assert(buffer.offset() == 200);
    assert(buffer.capacity() >= 200);
    
    std::cout << "[TEST] Capacity expansion test PASSED\n";
}

void test_error_handling() {
    std::cout << "[TEST] Error handling test...\n";
    
    // 외부 버퍼 용량 초과 테스트
    char small_buffer[10] = {0};
    packet_buffer buffer(small_buffer, 0, 10, data_option::none);
    
    try {
        buffer.append("This is too long for the buffer", 33);
        assert(false); // 예외가 발생해야 함
    } catch (const std::runtime_error& e) {
        std::cout << "[TEST] Expected error caught: " << e.what() << "\n";
    }
    
    std::cout << "[TEST] Error handling test PASSED\n";
}

int main() {
    std::cout << "=== Packet Buffer Tests ===\n";
    
    try {
        test_dynamic_buffer();
        test_external_buffer();
        test_move_semantics();
        test_capacity_expansion();
        test_error_handling();
        
        std::cout << "\n=== All tests PASSED! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Test failed: " << e.what() << "\n";
        return 1;
    }
}


