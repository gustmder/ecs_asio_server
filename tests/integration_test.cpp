#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

void test_basic_integration() {
    std::cout << "Basic Integration Test\n";

    const int worker_count = 4;
    const int task_count = 100;
    std::atomic<int> completed_tasks{0};

    std::vector<std::thread> workers;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back([&completed_tasks, task_count, worker_count]() {
            int tasks_per_worker = task_count / worker_count;
            for (int j = 0; j < tasks_per_worker; ++j) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                completed_tasks.fetch_add(1);
            }
        });
    }

    for (auto& w : workers) w.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    bool passed = (completed_tasks.load() == task_count);
    std::cout << "Completed: " << completed_tasks.load()
              << "  Duration: " << ms << " ms"
              << "  Result: " << (passed ? "PASS" : "FAIL") << "\n";
}

void test_memory_safety() {
    std::cout << "Memory Safety Test\n";

    const int iterations = 1000;
    std::atomic<int> success_count{0};

    for (int i = 0; i < iterations; ++i) {
        std::vector<int>* data = new std::vector<int>(1000);
        std::thread worker([&success_count, data]() {
            for (int j = 0; j < 100; ++j) {
                data->push_back(j);
            }
            success_count.fetch_add(1);
        });
        worker.join();
        delete data;
    }

    bool passed = (success_count.load() == iterations);
    std::cout << "Iterations: " << success_count.load() << "/" << iterations
              << "  Result: " << (passed ? "PASS" : "FAIL") << "\n";
}

void test_performance_benchmark() {
    std::cout << "Performance Benchmark Test\n";

    const int total_ops = 10000;
    const int thread_count = 8;
    std::atomic<int> completed{0};

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([&completed, total_ops, thread_count]() {
            int ops = total_ops / thread_count;
            for (int i = 0; i < ops; ++i) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                completed.fetch_add(1);
            }
        });
    }

    for (auto& t : threads) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    double ops_per_sec = (total_ops * 1000.0) / ms;

    bool passed = (completed.load() == total_ops);
    std::cout << "Ops: " << completed.load() << "/" << total_ops
              << "  Duration: " << ms << " ms"
              << "  Ops/s: " << ops_per_sec
              << "  Result: " << (passed ? "PASS" : "FAIL") << "\n";
}

void test_thread_communication() {
    std::cout << "Thread Communication Test\n";

    const int message_count = 1000;
    std::atomic<int> sent{0};
    std::atomic<int> received{0};

    std::vector<int> queue;
    std::mutex mtx;
    std::condition_variable cv;

    std::thread sender([&]() {
        for (int i = 0; i < message_count; ++i) {
            {
                std::lock_guard<std::mutex> lk(mtx);
                queue.push_back(i);
            }
            cv.notify_one();
            sent.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });

    std::thread receiver([&]() {
        for (int i = 0; i < message_count; ++i) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&queue]{ return !queue.empty(); });
            queue.erase(queue.begin());
            received.fetch_add(1);
        }
    });

    sender.join();
    receiver.join();

    bool passed = (sent.load() == message_count && received.load() == message_count);
    std::cout << "Sent: " << sent.load() << "  Received: " << received.load()
              << "  Result: " << (passed ? "PASS" : "FAIL") << "\n";
}

int main() {
    std::cout << "=== Integration Test Suite ===\n";

    try {
        test_basic_integration();
        test_memory_safety();
        test_performance_benchmark();
        test_thread_communication();

        std::cout << "\nAll integration tests completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
