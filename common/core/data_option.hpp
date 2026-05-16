
#pragma once

#include <cstdint>

namespace lemondory::core {

enum class data_option : std::uint8_t {
    none = 0,
    compression
};

} // namespace lemondory::core