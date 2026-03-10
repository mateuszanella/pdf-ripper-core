#include "core/compression/compression.hpp"

#include <zlib.h>

namespace ripper::core
{
    std::expected<std::vector<std::byte>, compression_error>
    compression::compress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(compression_error::invalid_input);
        }

        const uLongf maxSize = compressBound(static_cast<uLong>(input.size()));
        std::vector<std::byte> output(maxSize);

        uLongf compressedSize = maxSize;
        const int result = ::compress(
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
                    return std::unexpected(compression_error::memory_error);
                case Z_BUF_ERROR:
                    return std::unexpected(compression_error::buffer_too_small);
                default:
                    return std::unexpected(compression_error::compression_failed);
            }
        }

        output.resize(compressedSize);
        return output;
    }

    std::expected<std::vector<std::byte>, compression_error>
    compression::decompress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(compression_error::invalid_input);
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
                    return std::unexpected(compression_error::memory_error);
                case Z_DATA_ERROR:
                    return std::unexpected(compression_error::corrupted_data);
                default:
                    return std::unexpected(compression_error::decompression_failed);
            }
        }

        return std::unexpected(compression_error::buffer_too_small);
    }

    std::expected<std::vector<std::byte>, compression_error>
    compression::decompress(std::span<const std::byte> input, std::size_t expectedSize)
    {
        if (input.empty())
        {
            return std::unexpected(compression_error::invalid_input);
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
                    return std::unexpected(compression_error::memory_error);
                case Z_BUF_ERROR:
                    return std::unexpected(compression_error::buffer_too_small);
                case Z_DATA_ERROR:
                    return std::unexpected(compression_error::corrupted_data);
                default:
                    return std::unexpected(compression_error::decompression_failed);
            }
        }

        output.resize(decompressedSize);
        return output;
    }

    std::size_t compression::max_compressed_size(std::size_t inputSize) noexcept
    {
        return compressBound(static_cast<uLong>(inputSize));
    }
}
