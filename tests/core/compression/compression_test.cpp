#include "ripper/pdf/core/compression/compression.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <string_view>
#include <vector>

namespace
{
std::vector<std::byte> bytes_from_string(std::string_view text)
{
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());

    for (const char ch : text)
    {
        bytes.push_back(static_cast<std::byte>(ch));
    }

    return bytes;
}
} // namespace

namespace ripper::pdf::core
{
TEST_CASE("compression round-trips data", "[compression]")
{
    const auto input = bytes_from_string("A small PDF stream payload for round-trip testing.");

    const auto compressed = compression::compress(input);
    const auto decompressed = compression::decompress(compressed);

    REQUIRE(decompressed == input);
}

TEST_CASE("compression rejects empty input", "[compression]")
{
    const std::vector<std::byte> input;

    REQUIRE_THROWS_AS(compression::compress(input), logic_exception);
}

TEST_CASE("decompression rejects empty input", "[compression]")
{
    const std::vector<std::byte> input;

    REQUIRE_THROWS_AS(compression::decompress(input), logic_exception);
}

TEST_CASE("decompression rejects corrupted data", "[compression]")
{
    const std::vector<std::byte> input{
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
    };

    REQUIRE_THROWS_AS(compression::decompress(input), parse_exception);
}
} // namespace ripper::pdf::core
