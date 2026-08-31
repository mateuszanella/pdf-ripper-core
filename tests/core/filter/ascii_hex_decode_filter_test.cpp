#include "ripper/pdf/core/filter/ascii_hex_decode_filter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
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

TEST_CASE("ascii_hex_decode_filter round-trip", "[filter][asciihex]")
{
    ascii_hex_decode_filter filter;

    SECTION("small data round-trips correctly")
    {
        auto input = b("Hello, PDF!");
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }

    SECTION("empty input produces empty output")
    {
        auto input = b("");
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded.empty());
    }

    SECTION("binary data round-trips correctly")
    {
        std::vector<std::byte> input(256);
        for (int i = 0; i < 256; ++i)
            input[static_cast<std::size_t>(i)] = static_cast<std::byte>(i);
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }

    SECTION("encoded output ends with '>'")
    {
        auto input = b("test");
        auto encoded = filter.encode(input);
        REQUIRE(encoded.back() == std::byte{'>'});
    }
}

TEST_CASE("ascii_hex_decode_filter decode", "[filter][asciihex]")
{
    ascii_hex_decode_filter filter;

    SECTION("ignores whitespace")
    {
        auto encoded = b("48 65\r\n6C 6C\t6F>");
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == b("Hello"));
    }

    SECTION("stops at '>'")
    {
        auto encoded = b("41>42");
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == b("A"));
    }

    SECTION("odd hex digits pad with zero")
    {
        auto encoded = b("abc>");
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded.size() == 2);
        REQUIRE(decoded[0] == std::byte{0xAB});
        REQUIRE(decoded[1] == std::byte{0xC0});
    }

    SECTION("lowercase hex")
    {
        auto encoded = b("6162>");
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == b("ab"));
    }
}

} // namespace ripper::pdf::core
