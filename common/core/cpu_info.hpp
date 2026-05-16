#pragma once

#include <thread>

namespace lemondory::core {

inline int cpu_core_count() {
    return static_cast<int>(std::thread::hardware_concurrency());
}

} // namespace lemondory::core
