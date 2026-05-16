#pragma once

#include <atomic>
#include <thread>

namespace lemondory::core {

class spin_lock {
public:
    spin_lock() = default;
    spin_lock(const spin_lock&) = delete;
    spin_lock& operator=(const spin_lock&) = delete;

    void lock() {
        while (lock_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    void unlock() {
        lock_.clear(std::memory_order_release);
    }

    class guard {
    public:
        explicit guard(spin_lock& spin) : lock_(spin) { lock_.lock(); }
        ~guard() { lock_.unlock(); }

        guard(const guard&) = delete;
        guard& operator=(const guard&) = delete;

    private:
        spin_lock& lock_;
    };

private:
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};

} // namespace net