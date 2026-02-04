#pragma once

#include <cstdint>

namespace Ripper::Core
{
    enum class CompressionError : std::uint8_t
    {
        InvalidInput,
        BufferTooSmall,
        CompressionFailed,
        DecompressionFailed,
        CorruptedData,
        MemoryError
    };
}
