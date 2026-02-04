#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <vector>

#include "Core/Errors/Compression/CompressionError.hpp"

namespace Ripper::Core
{
    /**
     * @brief Provides compression/decompression for PDF stream data.
     *
     * Uses zlib-ng under the hood for FlateDecode filter support.
     */
    class Compression
    {
    public:
        /**
         * @brief Compresses raw data using DEFLATE algorithm.
         * @param input Raw uncompressed data
         * @return Compressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, CompressionError>
            Compress(std::span<const std::byte> input);

        /**
         * @brief Decompresses DEFLATE-compressed data.
         * @param input Compressed data
         * @return Decompressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, CompressionError>
            Decompress(std::span<const std::byte> input);

        /**
         * @brief Decompresses data with a known output size (more efficient).
         * @param input Compressed data
         * @param expectedSize Expected size of decompressed data
         * @return Decompressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, CompressionError>
            Decompress(std::span<const std::byte> input, std::size_t expectedSize);

        /**
         * @brief Calculates maximum size needed for compression buffer.
         * @param inputSize Size of data to compress
         * @return Maximum compressed size
         */
        [[nodiscard]] static std::size_t MaxCompressedSize(std::size_t inputSize) noexcept;
    };
}
