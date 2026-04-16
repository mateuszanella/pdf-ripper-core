#include "core/compression/compression.hpp"

#include <zlib.h>
#include <string>

#include "core/error_builder.hpp"

namespace ripper::core
{
    std::expected<std::vector<std::byte>, error>
    compression::compress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::compression_invalid_input)
                                       .with_component(error_component::compression)
                                       .with_field("input")
                                       .with_expected("non-empty buffer")
                                       .with_actual("empty")
                                       .with_message("Compression input is empty")
                                       .build());
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
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_memory_error)
                                               .with_component(error_component::compression)
                                               .with_field("compress")
                                               .with_message("Compression failed due to memory error")
                                               .build());
                case Z_BUF_ERROR:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_buffer_too_small)
                                               .with_component(error_component::compression)
                                               .with_field("output_buffer")
                                               .with_expected(">= compressBound(input_size)")
                                               .with_actual(std::to_string(maxSize))
                                               .with_message("Compression output buffer too small")
                                               .build());
                default:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_failed)
                                               .with_component(error_component::compression)
                                               .with_field("compress")
                                               .with_message("Compression failed")
                                               .build());
            }
        }

        output.resize(compressedSize);
        return output;
    }

    std::expected<std::vector<std::byte>, error>
    compression::decompress(std::span<const std::byte> input)
    {
        if (input.empty())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::compression_invalid_input)
                                       .with_component(error_component::compression)
                                       .with_field("input")
                                       .with_expected("non-empty buffer")
                                       .with_actual("empty")
                                       .with_message("Decompression input is empty")
                                       .build());
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
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_memory_error)
                                               .with_component(error_component::compression)
                                               .with_field("decompress")
                                               .with_message("Decompression failed due to memory error")
                                               .build());
                case Z_DATA_ERROR:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_compressed_data)
                                               .with_component(error_component::compression)
                                               .with_field("compressed_stream")
                                               .with_message("Compressed stream is corrupted")
                                               .build());
                default:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::decompression_failed)
                                               .with_component(error_component::compression)
                                               .with_field("decompress")
                                               .with_message("Decompression failed")
                                               .build());
            }
        }

        return std::unexpected(error_builder::create()
                                   .with_code(error_code::compression_buffer_too_small)
                                   .with_component(error_component::compression)
                                   .with_field("output_buffer")
                                   .with_expected("larger buffer after retries")
                                   .with_actual("insufficient")
                                   .with_message("Decompression buffer too small after retries")
                                   .build());
    }

    std::expected<std::vector<std::byte>, error>
    compression::decompress(std::span<const std::byte> input, std::size_t expectedSize)
    {
        if (input.empty())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::compression_invalid_input)
                                       .with_component(error_component::compression)
                                       .with_field("input")
                                       .with_expected("non-empty buffer")
                                       .with_actual("empty")
                                       .with_message("Decompression input is empty")
                                       .build());
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
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_memory_error)
                                               .with_component(error_component::compression)
                                               .with_field("decompress")
                                               .with_message("Decompression failed due to memory error")
                                               .build());
                case Z_BUF_ERROR:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::compression_buffer_too_small)
                                               .with_component(error_component::compression)
                                               .with_field("output_buffer")
                                               .with_expected(std::to_string(expectedSize))
                                               .with_actual("too small")
                                               .with_message("Provided decompression buffer is too small")
                                               .build());
                case Z_DATA_ERROR:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_compressed_data)
                                               .with_component(error_component::compression)
                                               .with_field("compressed_stream")
                                               .with_message("Compressed stream is corrupted")
                                               .build());
                default:
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::decompression_failed)
                                               .with_component(error_component::compression)
                                               .with_field("decompress")
                                               .with_message("Decompression failed")
                                               .build());
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
