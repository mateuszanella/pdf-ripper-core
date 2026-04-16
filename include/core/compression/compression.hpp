#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <vector>

#include "core/error.hpp"

namespace ripper::core
{
    /**
     * @brief Provides compression/decompression for pdf stream data.
     *
     * Uses zlib-ng under the hood for FlateDecode filter support.
     */
    class compression
    {
    public:
        /**
         * @brief Compresses raw data using DEFLATE algorithm.
         * @param input Raw uncompressed data
         * @return Compressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, error>
            compress(std::span<const std::byte> input);

        /**
         * @brief Decompresses DEFLATE-compressed data.
         * @param input Compressed data
         * @return Decompressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, error>
            decompress(std::span<const std::byte> input);

        /**
         * @brief Decompresses data with a known output size (more efficient).
         * @param input Compressed data
         * @param expectedSize Expected size of decompressed data
         * @return Decompressed data or error
         */
        [[nodiscard]] static std::expected<std::vector<std::byte>, error>
            decompress(std::span<const std::byte> input, std::size_t expectedSize);

        /**
         * @brief Calculates maximum size needed for compression buffer.
         * @param inputSize size of data to compress
         * @return Maximum compressed size
         */
        [[nodiscard]] static std::size_t max_compressed_size(std::size_t inputSize) noexcept;
    };
}
