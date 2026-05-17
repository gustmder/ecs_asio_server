#pragma once
// spdlog 1.15.3 + Apple Clang 17 호환성 버그로 std::cout fallback 사용 중
// (spdlog bundled fmt의 basic_spdata_ 미정의 이슈)
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace log_impl {

enum class Level { TRACE = 0, DEBUG, INFO, WARN, ERROR, CRITICAL };

inline Level& global_level() {
    static Level lv = Level::DEBUG;
    return lv;
}

inline std::mutex& log_mutex() {
    static std::mutex mtx;
    return mtx;
}

inline void write(Level level, const std::string& msg) {
    if (level < global_level()) return;

    static const char* names[] = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "CRIT "};
    static const char* colors[] = {"\033[37m","\033[36m","\033[32m","\033[33m","\033[31m","\033[35m"};
    static const char* rst = "\033[0m";

    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::lock_guard<std::mutex> lk(log_mutex());
    std::cout << std::put_time(std::localtime(&t), "[%H:%M:%S")
              << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] "
              << colors[static_cast<int>(level)]
              << '[' << names[static_cast<int>(level)] << ']'
              << rst << ' ' << msg << '\n';
}

} // namespace log_impl

inline void log_init(log_impl::Level level = log_impl::Level::DEBUG) {
    log_impl::global_level() = level;
}

#define LOGI(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::INFO,     _s.str()); } while(0)
#define LOGW(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::WARN,     _s.str()); } while(0)
#define LOGE(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::ERROR,    _s.str()); } while(0)
#define LOGD(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::DEBUG,    _s.str()); } while(0)
#define LOGT(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::TRACE,    _s.str()); } while(0)
#define LOGC(...) do { std::ostringstream _s; _s << __VA_ARGS__; log_impl::write(log_impl::Level::CRITICAL, _s.str()); } while(0)
