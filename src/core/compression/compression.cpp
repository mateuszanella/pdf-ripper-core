#include "core/compression/compression.hpp"

#include <zlib.h>
#include <string>

#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    std::vector<std::byte>
    compression::compress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            throw logic_exception{"Compression input is empty"};
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
                    throw io_exception{"Compression failed due to memory error"};
                case Z_BUF_ERROR:
                    throw io_exception{"Compression output buffer too small"};
                default:
                    throw io_exception{"Compression failed"};
            }
        }

        output.resize(compressedSize);
        return output;
    }

    std::vector<std::byte>
    compression::decompress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            throw logic_exception{"Decompression input is empty"};
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
                    throw io_exception{"Decompression failed due to memory error"};
                case Z_DATA_ERROR:
                    throw parse_exception{"Compressed stream is corrupted"};
                default:
                    throw io_exception{"Decompression failed"};
            }
        }

        throw io_exception{"Decompression buffer too small after retries"};
    }

    std::vector<std::byte>
    compression::decompress(std::span<const std::byte> input, std::size_t expectedSize)
    {
        if (input.empty())
        {
            throw logic_exception{"Decompression input is empty"};
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
                    throw io_exception{"Decompression failed due to memory error"};
                case Z_BUF_ERROR:
                    throw io_exception{"Provided decompression buffer is too small"};
                case Z_DATA_ERROR:
                    throw parse_exception{"Compressed stream is corrupted"};
                default:
                    throw io_exception{"Decompression failed"};
            }
        }

        output.resize(decompressedSize);
        return output;
    }

    std::size_t compression::max_compressed_size(std::size_t inputSize)
    {
        return compressBound(static_cast<uLong>(inputSize));
    }
}
