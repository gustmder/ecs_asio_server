#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

inline void log_init(spdlog::level::level_enum level = spdlog::level::debug) {
    auto logger = spdlog::stdout_color_mt("server");
    logger->set_level(level);
    logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_default_logger(logger);
    spdlog::set_level(level);
}

#define LOGI(...) spdlog::info(__VA_ARGS__)
#define LOGW(...) spdlog::warn(__VA_ARGS__)
#define LOGE(...) spdlog::error(__VA_ARGS__)
#define LOGD(...) spdlog::debug(__VA_ARGS__)
#define LOGT(...) spdlog::trace(__VA_ARGS__)
#define LOGC(...) spdlog::critical(__VA_ARGS__)
