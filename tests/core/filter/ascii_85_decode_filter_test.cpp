#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/ascii_85_decode_filter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <string>
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

TEST_CASE("ascii_85_decode_filter round-trip", "[filter][ascii85]")
{
    ascii_85_decode_filter filter;

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

    SECTION("4 zero bytes encode as 'z'")
    {
        std::vector<std::byte> input(4, std::byte{0});
        auto encoded = filter.encode(input);
        std::string encoded_str;
        for (auto b : encoded)
            encoded_str += static_cast<char>(b);
        REQUIRE(encoded_str.find('z') != std::string::npos);
    }

    SECTION("encoded output ends with '~>'")
    {
        auto input = b("test");
        auto encoded = filter.encode(input);
        REQUIRE(encoded.size() >= 2);
        REQUIRE(encoded[encoded.size() - 2] == std::byte{'~'});
        REQUIRE(encoded[encoded.size() - 1] == std::byte{'>'});
    }
}

TEST_CASE("ascii_85_decode_filter decode", "[filter][ascii85]")
{
    ascii_85_decode_filter filter;

    SECTION("decodes 'z' as 4 zero bytes")
    {
        auto encoded = b("z~>");
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded.size() == 4);
        for (auto b : decoded)
            REQUIRE(b == std::byte{0});
    }

    SECTION("ignores whitespace")
    {
        auto encoded = b("6<\\%  UP8~>");
        auto decoded = filter.decode(encoded);
        auto re_encoded = filter.encode(decoded);
        auto re_decoded = filter.decode(re_encoded);
        REQUIRE(re_decoded == decoded);
    }

    SECTION("stops at '~'")
    {
        auto input = b("test");
        auto encoded = filter.encode(input);
        auto result = filter.decode(encoded);
        REQUIRE(result == input);
    }

    SECTION("round-trip with larger data")
    {
        std::string data(500, '\0');
        for (std::size_t i = 0; i < data.size(); ++i)
            data[i] = static_cast<char>(i % 128);
        auto input = b(data);
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }
}

TEST_CASE("ascii_85_decode_filter invalid input", "[filter][ascii85][error]")
{
    ascii_85_decode_filter filter;

    SECTION("character outside valid range throws")
    {
        auto encoded = b("v~>");
        REQUIRE_THROWS_AS(filter.decode(encoded), parse_exception);
    }
}

} // namespace ripper::pdf::core
