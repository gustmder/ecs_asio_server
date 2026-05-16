#include "thread_group.hpp"

namespace lemondory::core {

thread_group::~thread_group() {
    join();
}

// 지정된 개수만큼 스레드를 생성하고 주어진 작업 실행
void thread_group::start(std::size_t count, const std::function<void(std::size_t)>& task) {
    if (running_) return;
    running_ = true;

    threads_.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        threads_.emplace_back([this, task, i]() {
            task(i);
        });
    }
}

// 모든 스레드가 종료될 때까지 대기
void thread_group::join() {
    if (!running_) return;
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads_.clear();
    running_ = false;
}

} // namespace lemondory::core