#pragma once

#include <cstdint>

namespace ripper::core
{
    enum class compression_error : std::uint8_t
    {
        invalid_input,
        buffer_too_small,
        compression_failed,
        decompression_failed,
        corrupted_data,
        memory_error
    };
}
