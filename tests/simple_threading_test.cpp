#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

// 간단한 스레드 테스트
void test_basic_threading() {
    std::cout << "🧪 Basic Threading Test" << std::endl;
    
    std::atomic<int> counter{0};
    const int thread_count = 4;
    const int operations_per_thread = 1000;
    
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 여러 스레드에서 동시에 카운터 증가
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([&counter, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                counter.fetch_add(1);
            }
        });
    }
    
    // 모든 스레드 완료 대기
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Counter value: " << counter.load() << std::endl;
    std::cout << "Expected value: " << (thread_count * operations_per_thread) << std::endl;
    std::cout << "Duration: " << duration.count() << " microseconds" << std::endl;
    
    bool passed = (counter.load() == thread_count * operations_per_thread);
    std::cout << "Result: " << (passed ? "✅ PASS" : "❌ FAIL") << std::endl;
}

// 스레드 풀 테스트
void test_thread_pool() {
    std::cout << "\n🧪 Thread Pool Test" << std::endl;
    
    const int worker_count = 4;
    const int task_count = 100;
    std::atomic<int> completed_tasks{0};
    
    std::vector<std::thread> workers;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 워커 스레드 생성
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
    
    // 모든 워커 완료 대기
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

// 메모리 안전성 테스트
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

int main() {
    std::cout << "🧪 Simple Threading Test Suite" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        test_basic_threading();
        test_thread_pool();
        test_memory_safety();
        
        std::cout << "\n✅ All simple tests completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
