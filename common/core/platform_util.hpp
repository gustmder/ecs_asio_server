// platform_util.hpp
#pragma once

#include <cstdint>
#include <chrono>
#include <thread>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif

namespace lemondory::core {

// Cross-platform sleep function
inline void sleep_ms(std::uint32_t milliseconds) {
#if defined(_WIN32)
    ::Sleep(milliseconds);
#else
    ::usleep(milliseconds * 1000);
#endif
}

// Cross-platform tick count (milliseconds since epoch or boot)
inline std::uint64_t get_tick_ms() {
#if defined(_WIN32)
    return static_cast<std::uint64_t>(::GetTickCount64());
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<std::uint64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
#endif
}

} // namespace lemondory::core
