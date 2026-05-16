// common/timing.hpp
#pragma once
#include <chrono>
#include <mutex>
#include <spdlog/spdlog.h>

class timed_lock_guard {
public:
    explicit timed_lock_guard(std::mutex& m, const char* tag, std::chrono::milliseconds warn = std::chrono::milliseconds(1))
        : m_(m), tag_(tag), warn_(warn)
    {
        auto t0 = std::chrono::steady_clock::now();
        m_.lock();
        auto t1 = std::chrono::steady_clock::now();
        wait_ = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0);
        if (wait_ > warn_) {
            spdlog::warn("[lock-wait] {} waited {} us", tag_, wait_.count());
        }
    }
    ~timed_lock_guard(){ m_.unlock(); }

private:
    std::mutex& m_;
    const char* tag_;
    std::chrono::milliseconds warn_;
    std::chrono::microseconds wait_{0};
};