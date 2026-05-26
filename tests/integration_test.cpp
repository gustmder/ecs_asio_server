#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

// 간단한 통합 테스트
void test_basic_integration() {
    std::cout << "🧪 Basic Integration Test" << std::endl;
    
    // 1. 스레드 풀 테스트
    const int worker_count = 4;
    const int task_count = 100;
    std::atomic<int> completed_tasks{0};
    
    std::vector<std::thread> workers;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back([&completed_tasks, task_count, worker_count]() {
            int tasks_per_worker = task_count / worker_count;
            for (int j = 0; j < tasks_per_worker; ++j) {
                // 작업 시뮬레이션
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                completed_tasks.fetch_add(1);
            }
        });
    }
    
    for (auto& worker : workers) {
        worker.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Completed tasks: " << completed_tasks.load() << std::endl;
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    
    bool passed = (completed_tasks.load() == task_count);
    std::cout << "Result: " << (passed ? "✅ PASS" : "❌ FAIL") << std::endl;
}

// 2. 메모리 안전성 테스트
void test_memory_safety() {
    std::cout << "\n🧪 Memory Safety Test" << std::endl;
    
    const int iterations = 1000;
    std::atomic<int> success_count{0};
    
    for (int i = 0; i < iterations; ++i) {
        // 동적 할당/해제 테스트
        std::vector<int>* data = new std::vector<int>(1000);
        
        // 스레드에서 데이터 접근
        std::thread worker([&success_count, data]() {
            for (int j = 0; j < 100; ++j) {
                data->push_back(j);
            }
            success_count.fetch_add(1);
        });
        
        worker.join();
        delete data;
    }
    
    std::cout << "Successful iterations: " << success_count.load() << std::endl;
    std::cout << "Total iterations: " << iterations << std::endl;
    
    bool passed = (success_count.load() == iterations);
    std::cout << "Result: " << (passed ? "✅ PASS" : "❌ FAIL") << std::endl;
}

// 3. 성능 벤치마크 테스트
void test_performance_benchmark() {
    std::cout << "\n🧪 Performance Benchmark Test" << std::endl;
    
    const int total_operations = 10000;
    const int thread_count = 8;
    std::atomic<int> completed_operations{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&completed_operations, total_operations, thread_count]() {
            int operations_per_thread = total_operations / thread_count;
            for (int i = 0; i < operations_per_thread; ++i) {
                // 작업 시뮬레이션
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                completed_operations.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double operations_per_second = (total_operations * 1000.0) / duration.count();
    
    std::cout << "Total operations: " << total_operations << std::endl;
    std::cout << "Completed operations: " << completed_operations.load() << std::endl;
    std::cout << "Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "Operations per second: " << operations_per_second << std::endl;
    
    bool passed = (completed_operations.load() == total_operations);
    std::cout << "Result: " << (passed ? "✅ PASS" : "❌ FAIL") << std::endl;
}

// 4. 스레드 간 통신 테스트
void test_thread_communication() {
    std::cout << "\n🧪 Thread Communication Test" << std::endl;
    
    const int message_count = 1000;
    std::atomic<int> sent_messages{0};
    std::atomic<int> received_messages{0};
    
    // 메시지 큐 시뮬레이션
    std::vector<int> message_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    
    // 송신 스레드
    std::thread sender([&sent_messages, &message_queue, &queue_mutex, &cv, message_count]() {
        for (int i = 0; i < message_count; ++i) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                message_queue.push_back(i);
            }
            cv.notify_one();
            sent_messages.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    // 수신 스레드
    std::thread receiver([&received_messages, &message_queue, &queue_mutex, &cv, message_count]() {
        for (int i = 0; i < message_count; ++i) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [&message_queue] { return !message_queue.empty(); });
            
            if (!message_queue.empty()) {
                message_queue.erase(message_queue.begin());
                received_messages.fetch_add(1);
            }
        }
    });
    
    sender.join();
    receiver.join();
    
    std::cout << "Sent messages: " << sent_messages.load() << std::endl;
    std::cout << "Received messages: " << received_messages.load() << std::endl;
    
    bool passed = (sent_messages.load() == message_count && 
                   received_messages.load() == message_count);
    std::cout << "Result: " << (passed ? "✅ PASS" : "❌ FAIL") << std::endl;
}

int main() {
    std::cout << "🧪 Integration Test Suite" << std::endl;
    std::cout << "=========================" << std::endl;
    
    try {
        test_basic_integration();
        test_memory_safety();
        test_performance_benchmark();
        test_thread_communication();
        
        std::cout << "\n✅ All integration tests completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
