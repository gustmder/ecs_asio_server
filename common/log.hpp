#pragma once
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace log {

// 간단한 로깅 시스템
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    CRITICAL = 5
};

// 전역 로그 레벨
inline LogLevel& get_log_level() {
    static LogLevel level = LogLevel::INFO;
    return level;
}

inline void init() {
    get_log_level() = LogLevel::INFO;
    std::cout << "[LOG] Logging system initialized" << std::endl;
}

inline void set_level(LogLevel level) {
    get_log_level() = level;
}

inline std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

inline void log_message(LogLevel level, const std::string& message) {
    if (level < get_log_level()) return;
    
    const char* level_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL"};
    const char* level_colors[] = {"\033[37m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m"};
    const char* reset_color = "\033[0m";
    
    std::cout << "[" << get_timestamp() << "] "
              << level_colors[static_cast<int>(level)] 
              << "[" << level_names[static_cast<int>(level)] << "]"
              << reset_color << " " << message << std::endl;
}

} // namespace log

// 매크로: 간단한 로깅
#define LOGI(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::INFO, ss.str()); \
} while(0)

#define LOGW(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::WARN, ss.str()); \
} while(0)

#define LOGE(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::ERROR, ss.str()); \
} while(0)

#define LOGD(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::DEBUG, ss.str()); \
} while(0)

#define LOGT(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::TRACE, ss.str()); \
} while(0)

#define LOGC(...) do { \
    std::stringstream ss; \
    ss << __VA_ARGS__; \
    log::log_message(log::LogLevel::CRITICAL, ss.str()); \
} while(0)