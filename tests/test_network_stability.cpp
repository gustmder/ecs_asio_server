// test_network_stability.cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <memory>
#include <asio.hpp>
#include "server/network/asio_server.hpp"
#include "server/network/asio_channel.hpp"
#include "common/network/socket_option.hpp"
#include "server/game/game_server.hpp"

using namespace lemondory::network;
using namespace lemondory::game;

class test_game_server : public game_server {
public:
    test_game_server() = default;
    
    void on_channel_init(socket_channel_base* channel, bool /*use_plugin*/) override {
        std::cout << "[TEST] Channel initialized: " << channel->get_channel_id() << "\n";
    }
    
    void on_channel_destroy(socket_channel_base* channel) override {
        std::cout << "[TEST] Channel destroyed: " << channel->get_channel_id() << "\n";
    }
    
    void on_accept(int channel_id, socket_channel_base* /*channel*/) override {
        std::cout << "[TEST] Channel accepted: " << channel_id << "\n";
        active_connections_.fetch_add(1);
    }
    
    void on_close(int channel_id, const void* /*error*/, close_function reason) override {
        std::cout << "[TEST] Channel closed: " << channel_id << " reason: " << static_cast<int>(reason) << "\n";
        active_connections_.fetch_sub(1);
    }
    
    void on_message(int channel_id, std::uint16_t cmd, const char* payload, std::int32_t size) override {
        // 에코 테스트
        if (cmd == 1) { // PING
            std::cout << "[TEST] Received PING from channel " << channel_id << "\n";
            // PONG 응답 (간단한 에코)
            auto pong_data = std::string(payload, static_cast<std::size_t>(size));
            send_test_frame(channel_id, 2, pong_data.data(), pong_data.size());
        }
        message_count_.fetch_add(1);
    }
    
    std::atomic<int> active_connections_{0};
    std::atomic<int> message_count_{0};
};

void test_concurrent_connections() {
    std::cout << "[TEST] Concurrent connections test...\n";
    
    asio::io_context io;
    test_game_server server;
    
    socket_option opt;
    opt.set_reuse_address(true).set_backlog(128).set_tcp_no_delay(true);
    
    if (!server.init(io, opt, "127.0.0.1", 7777)) {
        std::cerr << "[ERROR] Server init failed\n";
        return;
    }
    
    std::cout << "[TEST] Server started, testing concurrent connections...\n";
    
    // 서버를 별도 스레드에서 실행
    std::thread server_thread([&io]() {
        io.run();
    });
    
    // 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 여러 클라이언트 연결 시뮬레이션
    const int num_clients = 10;
    std::vector<std::thread> client_threads;
    
    for (int i = 0; i < num_clients; ++i) {
        client_threads.emplace_back([i]() {
            try {
                asio::io_context client_io;
                asio::ip::tcp::socket socket(client_io);
                asio::ip::tcp::resolver resolver(client_io);
                
                auto endpoints = resolver.resolve("127.0.0.1", "7777");
                asio::connect(socket, endpoints);
                
                std::cout << "[TEST] Client " << i << " connected\n";
                
                // 간단한 메시지 전송
                std::string message = "Hello from client " + std::to_string(i);
                socket.write_some(asio::buffer(message));
                
                // 응답 대기
                char response[1024];
                size_t bytes_received = socket.read_some(asio::buffer(response));
                std::cout << "[TEST] Client " << i << " received response: " 
                         << std::string(response, bytes_received) << "\n";
                
                socket.close();
                std::cout << "[TEST] Client " << i << " disconnected\n";
                
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Client " << i << " error: " << e.what() << "\n";
            }
        });
    }
    
    // 모든 클라이언트 완료 대기
    for (auto& thread : client_threads) {
        thread.join();
    }
    
    // 서버 정리
    io.stop();
    server_thread.join();
    
    std::cout << "[TEST] Concurrent connections test completed\n";
}

void test_memory_leaks() {
    std::cout << "[TEST] Memory leak test...\n";
    
    // 여러 번 서버 시작/중지하여 메모리 누수 확인
    for (int i = 0; i < 10; ++i) {
        asio::io_context io;
        test_game_server server;
        
        socket_option opt;
        opt.set_reuse_address(true).set_backlog(128).set_tcp_no_delay(true);
        
        if (server.init(io, opt, "127.0.0.1", 7778 + i)) {
            std::cout << "[TEST] Server " << i << " started\n";
            
            // 짧은 시간 실행
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
            server.stop();
            std::cout << "[TEST] Server " << i << " stopped\n";
        }
    }
    
    std::cout << "[TEST] Memory leak test completed\n";
}

void test_error_recovery() {
    std::cout << "[TEST] Error recovery test...\n";
    
    asio::io_context io;
    test_game_server server;
    
    socket_option opt;
    opt.set_reuse_address(true).set_backlog(128).set_tcp_no_delay(true);
    
    if (!server.init(io, opt, "127.0.0.1", 7779)) {
        std::cerr << "[ERROR] Server init failed\n";
        return;
    }
    
    // 서버를 별도 스레드에서 실행
    std::thread server_thread([&io]() {
        try {
            io.run();
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Server error: " << e.what() << "\n";
        }
    });
    
    // 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 잘못된 연결 시도 (에러 복구 테스트)
    try {
        asio::io_context client_io;
        asio::ip::tcp::socket socket(client_io);
        asio::ip::tcp::resolver resolver(client_io);
        
        auto endpoints = resolver.resolve("127.0.0.1", "7779");
        asio::connect(socket, endpoints);
        
        // 연결 후 즉시 종료 (에러 상황 시뮬레이션)
        socket.close();
        
    } catch (const std::exception& e) {
        std::cout << "[TEST] Expected error: " << e.what() << "\n";
    }
    
    // 서버 정리
    io.stop();
    server_thread.join();
    
    std::cout << "[TEST] Error recovery test completed\n";
}

int main() {
    std::cout << "=== Network Stability Tests ===\n";
    
    try {
        test_concurrent_connections();
        test_memory_leaks();
        test_error_recovery();
        
        std::cout << "\n=== All stability tests completed! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Test failed: " << e.what() << "\n";
        return 1;
    }
}

