#pragma once

#include <vector>
#include <thread>
#include <functional>
#include <atomic>

namespace lemondory::core {

// 간단한 스레드 그룹 실행기
class thread_group {
public:
    thread_group() = default;
    thread_group(const thread_group&) = delete;
    thread_group& operator=(const thread_group&) = delete;
    ~thread_group();

    // 지정한 개수만큼 스레드를 생성하고 작업을 실행
    void start(std::size_t count, const std::function<void(std::size_t)>& task);

    // 모든 스레드가 종료될 때까지 대기
    void join();

    // 현재 실행 중인 스레드 수 반환
    std::size_t size() const { return threads_.size(); }

private:
    std::vector<std::thread> threads_;
    std::atomic<bool> running_{false};
};

} // namespace lemondory::core


	