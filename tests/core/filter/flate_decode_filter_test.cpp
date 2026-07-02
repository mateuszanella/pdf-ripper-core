#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/flate_decode_filter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{

std::vector<std::byte> b(std::string_view sv)
{
    std::vector<std::byte> v;
    v.reserve(sv.size());
    for (char c : sv)
        v.push_back(static_cast<std::byte>(c));
    return v;
}

} // namespace

TEST_CASE("flate_decode_filter round-trip", "[filter][flate]")
{
    flate_decode_filter filter;

    SECTION("small data round-trips correctly")
    {
        auto input = b("Hello, PDF!");
        auto compressed = filter.encode(input);
        auto decompressed = filter.decode(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("empty input produces empty output")
    {
        auto input = b("");
        auto compressed = filter.encode(input);
        REQUIRE(compressed.empty());
        auto decompressed = filter.decode(compressed);
        REQUIRE(decompressed.empty());
    }

    SECTION("binary data round-trips correctly")
    {
        std::vector<std::byte> input(256);
        for (int i = 0; i < 256; ++i)
            input[static_cast<std::size_t>(i)] = static_cast<std::byte>(i);
        auto compressed = filter.encode(input);
        auto decompressed = filter.decode(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("repeated data compresses well")
    {
        auto input = b(std::string(1000, 'A'));
        auto compressed = filter.encode(input);
        REQUIRE(compressed.size() < input.size());
        auto decompressed = filter.decode(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("large data round-trips correctly")
    {
        std::vector<std::byte> input(1024 * 1024);
        for (std::size_t i = 0; i < input.size(); ++i)
            input[i] = static_cast<std::byte>(i % 256);
        auto compressed = filter.encode(input);
        auto decompressed = filter.decode(compressed);
        REQUIRE(decompressed == input);
    }
}

TEST_CASE("flate_decode_filter corrupted data", "[filter][flate][error]")
{
    flate_decode_filter filter;

    SECTION("garbage data throws parse_exception")
    {
        auto garbage = b("this is not compressed data");
        REQUIRE_THROWS_AS(filter.decode(garbage), parse_exception);
    }

    SECTION("truncated compressed data throws parse_exception")
    {
        auto input = b("Hello, PDF!");
        auto compressed = filter.encode(input);
        auto truncated =
            std::vector<std::byte>{compressed.begin(), compressed.begin() + compressed.size() / 2};
        REQUIRE_THROWS_AS(filter.decode(truncated), parse_exception);
    }
}

} // namespace ripper::pdf::core
