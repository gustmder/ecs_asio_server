// benchmark_performance.cpp
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <asio.hpp>
#include "../core/packet_buffer.hpp"
#include "../network/asio_server.hpp"
#include "../network/socket_option.hpp"

using namespace lemondory::core;
using namespace lemondory::network;

class benchmark_server : public net_handler {
public:
    void on_channel_init(socket_channel_base* /*channel*/, bool /*use_plugin*/) override {}
    void on_channel_destroy(socket_channel_base* /*channel*/) override {}
    void on_accept(int /*channel_id*/, socket_channel_base* /*channel*/) override {
        active_connections_.fetch_add(1);
    }
    void on_close(int /*channel_id*/, const void* /*error*/, close_function /*reason*/) override {
        active_connections_.fetch_sub(1);
    }
    void on_tick() override {}
    void on_message(int /*channel_id*/, std::uint16_t /*cmd*/, const char* /*payload*/, std::int32_t /*size*/) override {
        message_count_.fetch_add(1);
    }
    
    std::atomic<int> active_connections_{0};
    std::atomic<int> message_count_{0};
};

void benchmark_packet_buffer() {
    std::cout << "[BENCHMARK] Packet Buffer Performance...\n";
    
    const int iterations = 1000000;
    const std::string test_data = "Hello, World! This is a test message for benchmarking.";
    
    // 동적 버퍼 테스트
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        packet_buffer buffer(1024, data_option::none);
        buffer.append(test_data);
        buffer.set_offset(5);
        buffer.append("Additional data", 15);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "[BENCHMARK] Dynamic buffer: " << iterations << " operations in " 
              << duration.count() << " microseconds\n";
    std::cout << "[BENCHMARK] Average: " << (double)duration.count() / iterations << " μs per operation\n";
}

void benchmark_concurrent_operations() {
    std::cout << "[BENCHMARK] Concurrent Operations...\n";
    
    const int num_threads = 4;
    const int operations_per_thread = 100000;
    std::atomic<int> total_operations{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&total_operations]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                packet_buffer buffer(512, data_option::none);
                buffer.append("Thread operation", 16);
                total_operations.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "[BENCHMARK] " << total_operations.load() << " concurrent operations in " 
              << duration.count() << " ms\n";
    std::cout << "[BENCHMARK] Throughput: " << (double)total_operations.load() / duration.count() 
              << " operations/ms\n";
}

void benchmark_network_throughput() {
    std::cout << "[BENCHMARK] Network Throughput...\n";
    
    asio::io_context io;
    benchmark_server server;
    
    socket_option opt;
    opt.set_reuse_address(true).set_backlog(128).set_tcp_no_delay(true);
    
    auto server_impl = std::make_shared<asio_server>(io);
    if (!server_impl->init(&server, opt, 100, 2)) {
        std::cerr << "[ERROR] Server init failed\n";
        return;
    }
    
    if (!server_impl->listen("127.0.0.1", 8888)) {
        std::cerr << "[ERROR] Server listen failed\n";
        return;
    }
    
    // 서버를 별도 스레드에서 실행
    std::thread server_thread([&io]() {
        io.run();
    });
    
    // 잠시 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 클라이언트 연결 및 메시지 전송
    const int num_clients = 10;
    const int messages_per_client = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> client_threads;
    for (int i = 0; i < num_clients; ++i) {
        client_threads.emplace_back([i]() {
            try {
                asio::io_context client_io;
                asio::ip::tcp::socket socket(client_io);
                asio::ip::tcp::resolver resolver(client_io);
                
                auto endpoints = resolver.resolve("127.0.0.1", "8888");
                asio::connect(socket, endpoints);
                
                for (int j = 0; j < messages_per_client; ++j) {
                    std::string message = "Message " + std::to_string(j) + " from client " + std::to_string(i);
                    socket.write_some(asio::buffer(message));
                }
                
                socket.close();
                
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Client " << i << " error: " << e.what() << "\n";
            }
        });
    }
    
    // 모든 클라이언트 완료 대기
    for (auto& thread : client_threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 서버 정리
    io.stop();
    server_thread.join();
    
    int total_messages = num_clients * messages_per_client;
    std::cout << "[BENCHMARK] " << total_messages << " messages in " << duration.count() << " ms\n";
    std::cout << "[BENCHMARK] Throughput: " << (double)total_messages / duration.count() << " messages/ms\n";
    std::cout << "[BENCHMARK] Active connections: " << server.active_connections_.load() << "\n";
    std::cout << "[BENCHMARK] Messages processed: " << server.message_count_.load() << "\n";
}

void benchmark_memory_usage() {
    std::cout << "[BENCHMARK] Memory Usage...\n";
    
    const int num_buffers = 10000;
    std::vector<std::unique_ptr<packet_buffer>> buffers;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 버퍼 생성
    for (int i = 0; i < num_buffers; ++i) {
        auto buffer = std::make_unique<packet_buffer>(1024, data_option::none);
        buffer->append("Test data for memory benchmark", 30);
        buffers.push_back(std::move(buffer));
    }
    
    auto mid = std::chrono::high_resolution_clock::now();
    
    // 버퍼 해제
    buffers.clear();
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto creation_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
    auto destruction_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
    
    std::cout << "[BENCHMARK] Created " << num_buffers << " buffers in " << creation_time.count() << " μs\n";
    std::cout << "[BENCHMARK] Destroyed " << num_buffers << " buffers in " << destruction_time.count() << " μs\n";
    std::cout << "[BENCHMARK] Average creation: " << (double)creation_time.count() / num_buffers << " μs per buffer\n";
    std::cout << "[BENCHMARK] Average destruction: " << (double)destruction_time.count() / num_buffers << " μs per buffer\n";
}

int main() {
    std::cout << "=== Performance Benchmark ===\n";
    
    try {
        benchmark_packet_buffer();
        std::cout << "\n";
        
        benchmark_concurrent_operations();
        std::cout << "\n";
        
        benchmark_memory_usage();
        std::cout << "\n";
        
        benchmark_network_throughput();
        std::cout << "\n";
        
        std::cout << "=== Benchmark completed! ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Benchmark failed: " << e.what() << "\n";
        return 1;
    }
}

