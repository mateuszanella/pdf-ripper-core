#pragma once

#include "core/exceptions/exception.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace ripper::pdf::core
{
/**
 * @brief Provides compression/decompression for pdf stream data.
 *
 * Uses zlib-ng under the hood for FlateDecode filter support.
 *
 * @todo rewrite all of this when i actually use it
 */
class compression
{
public:
    /**
     * @brief Compresses raw data using DEFLATE algorithm.
     * @param input Raw uncompressed data
     */
    [[nodiscard]] static std::vector<std::byte> compress(std::span<const std::byte> input);

    /**
     * @brief Decompresses DEFLATE-compressed data.
     * @param input Compressed data
     */
    [[nodiscard]] static std::vector<std::byte> decompress(std::span<const std::byte> input);

    /**
     * @brief Decompresses data with a known output size (more efficient).
     * @param input Compressed data
     * @param expectedSize Expected size of decompressed data
     */
    [[nodiscard]] static std::vector<std::byte> decompress(std::span<const std::byte> input,
                                                           std::size_t expectedSize);

    /**
     * @brief Calculates maximum size needed for compression buffer.
     * @param inputSize size of data to compress
     * @return Maximum compressed size
     */
    [[nodiscard]] static std::size_t max_compressed_size(std::size_t inputSize);
};
} // namespace ripper::pdf::core
