#include "ripper/pdf/core/serializer/header/default_header_serializer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace ripper::pdf::core
{
namespace
{
std::string bytes_to_string(const std::vector<std::byte>& bytes)
{
    std::string result;
    result.reserve(bytes.size());
    for (auto b : bytes)
        result += static_cast<char>(b);
    return result;
}
} // namespace

TEST_CASE("default_header_serializer serializes PDF-1.7 header", "[serializer][header]")
{
    const header hdr{"1.7"};
    default_header_serializer ser;

    const auto result = ser.serialize(hdr);

    REQUIRE(bytes_to_string(result) == "%PDF-1.7\n");
}

TEST_CASE("default_header_serializer serializes PDF-2.0 header", "[serializer][header]")
{
    const header hdr{"2.0"};
    default_header_serializer ser;

    const auto result = ser.serialize(hdr);

    REQUIRE(bytes_to_string(result) == "%PDF-2.0\n");
}

TEST_CASE("default_header_serializer serializes header with version containing subminor",
          "[serializer][header]")
{
    const header hdr{"1.4"};
    default_header_serializer ser;

    const auto result = ser.serialize(hdr);

    REQUIRE(bytes_to_string(result) == "%PDF-1.4\n");
}

TEST_CASE("default_header_serializer uses custom line break character", "[serializer][header]")
{
    const header hdr{"1.7"};
    default_header_serializer ser;
    ser.set_line_break_character('\r');

    const auto result = ser.serialize(hdr);

    REQUIRE(bytes_to_string(result) == "%PDF-1.7\r");
}

TEST_CASE("default_header_serializer uses CRLF line break", "[serializer][header]")
{
    const header hdr{"2.0"};
    default_header_serializer ser;
    ser.set_line_break_character('\n');

    const auto result = ser.serialize(hdr);

    REQUIRE(bytes_to_string(result) == "%PDF-2.0\n");
}

TEST_CASE("default_header_serializer throws on empty version", "[serializer][header][corrupted]")
{
    const header hdr{""};
    default_header_serializer ser;

    REQUIRE_THROWS_WITH(ser.serialize(hdr),
                        Catch::Matchers::ContainsSubstring("Header version cannot be empty"));
}
} // namespace ripper::pdf::core
