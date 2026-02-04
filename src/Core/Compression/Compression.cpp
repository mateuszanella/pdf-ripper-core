#include "Core/Compression/Compression.hpp"

#include <zlib.h>

namespace Ripper::Core
{
    std::expected<std::vector<std::byte>, CompressionError>
    Compression::Compress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(CompressionError::InvalidInput);
        }

        const uLongf maxSize = compressBound(static_cast<uLong>(input.size()));
        std::vector<std::byte> output(maxSize);

        uLongf compressedSize = maxSize;
        const int result = compress(
            reinterpret_cast<Bytef*>(output.data()),
            &compressedSize,
            reinterpret_cast<const Bytef*>(input.data()),
            static_cast<uLong>(input.size())
        );

        if (result != Z_OK)
        {
            switch (result)
            {
                case Z_MEM_ERROR:
                    return std::unexpected(CompressionError::MemoryError);
                case Z_BUF_ERROR:
                    return std::unexpected(CompressionError::BufferTooSmall);
                default:
                    return std::unexpected(CompressionError::CompressionFailed);
            }
        }

        output.resize(compressedSize);
        return output;
    }

    std::expected<std::vector<std::byte>, CompressionError>
    Compression::Decompress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(CompressionError::InvalidInput);
        }

        // Start with estimated size and grow if needed
        std::size_t outputSize = input.size() * 4;
        constexpr std::size_t kMaxAttempts = 5;

        for (std::size_t attempt = 0; attempt < kMaxAttempts; ++attempt)
        {
            std::vector<std::byte> output(outputSize);
            uLongf decompressedSize = static_cast<uLongf>(outputSize);

            const int result = uncompress(
                reinterpret_cast<Bytef*>(output.data()),
                &decompressedSize,
                reinterpret_cast<const Bytef*>(input.data()),
                static_cast<uLong>(input.size())
            );

            if (result == Z_OK)
            {
                output.resize(decompressedSize);
                return output;
            }

            if (result == Z_BUF_ERROR)
            {
                outputSize *= 2;
                continue;
            }

            switch (result)
            {
                case Z_MEM_ERROR:
                    return std::unexpected(CompressionError::MemoryError);
                case Z_DATA_ERROR:
                    return std::unexpected(CompressionError::CorruptedData);
                default:
                    return std::unexpected(CompressionError::DecompressionFailed);
            }
        }

        return std::unexpected(CompressionError::BufferTooSmall);
    }

    std::expected<std::vector<std::byte>, CompressionError>
    Compression::Decompress(std::span<const std::byte> input, std::size_t expectedSize)
    {
        if (input.empty())
        {
            return std::unexpected(CompressionError::InvalidInput);
        }

        std::vector<std::byte> output(expectedSize);
        uLongf decompressedSize = static_cast<uLongf>(expectedSize);

        const int result = uncompress(
            reinterpret_cast<Bytef*>(output.data()),
            &decompressedSize,
            reinterpret_cast<const Bytef*>(input.data()),
            static_cast<uLong>(input.size())
        );

        if (result != Z_OK)
        {
            switch (result)
            {
                case Z_MEM_ERROR:
                    return std::unexpected(CompressionError::MemoryError);
                case Z_BUF_ERROR:
                    return std::unexpected(CompressionError::BufferTooSmall);
                case Z_DATA_ERROR:
                    return std::unexpected(CompressionError::CorruptedData);
                default:
                    return std::unexpected(CompressionError::DecompressionFailed);
            }
        }

        output.resize(decompressedSize);
        return output;
    }

    std::size_t Compression::MaxCompressedSize(std::size_t inputSize) noexcept
    {
        return compressBound(static_cast<uLong>(inputSize));
    }
}
