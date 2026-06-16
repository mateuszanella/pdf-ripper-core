#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/header/header_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
std::vector<std::byte> make_bytes(std::string_view s)
{
    std::vector<std::byte> bytes(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        bytes[i] = std::byte{static_cast<unsigned char>(s[i])};
    }
    return bytes;
}
} // namespace

TEST_CASE("header_parser parses PDF-1.7 header", "[parser][header]")
{
    auto data = make_bytes("%PDF-1.7\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    const auto result = parser.parse();

    REQUIRE(result.version() == "1.7");
}

TEST_CASE("header_parser parses PDF-2.0 header", "[parser][header]")
{
    auto data = make_bytes("%PDF-2.0\r\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    const auto result = parser.parse();

    REQUIRE(result.version() == "2.0");
}

TEST_CASE("header_parser parses header with binary prefix before signature", "[parser][header]")
{
    auto data = make_bytes("%binary stuff %PDF-1.4\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    const auto result = parser.parse();

    REQUIRE(result.version() == "1.4");
}

TEST_CASE("header_parser parses header with trailing content", "[parser][header]")
{
    auto data = make_bytes("%PDF-1.7 trailing junk that is ignored\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    const auto result = parser.parse();

    REQUIRE(result.version() == "1.7");
}

TEST_CASE("header_parser throws on empty file", "[parser][header][corrupted]")
{
    auto data = make_bytes("");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(), Catch::Matchers::ContainsSubstring("empty"));
}

TEST_CASE("header_parser throws on missing PDF signature", "[parser][header][corrupted]")
{
    auto data = make_bytes("%not a pdf header\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(), Catch::Matchers::ContainsSubstring("Missing PDF header"));
}

TEST_CASE("header_parser throws on missing version after %PDF-", "[parser][header][corrupted]")
{
    auto data = make_bytes("%PDF-\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(),
                        Catch::Matchers::ContainsSubstring("Invalid PDF header version"));
}

TEST_CASE("header_parser throws on version without dot", "[parser][header][corrupted]")
{
    auto data = make_bytes("%PDF-1\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(),
                        Catch::Matchers::ContainsSubstring("Invalid PDF header version format"));
}

TEST_CASE("header_parser throws on version with trailing dot", "[parser][header][corrupted]")
{
    auto data = make_bytes("%PDF-1.\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(),
                        Catch::Matchers::ContainsSubstring("Invalid PDF header version format"));
}

TEST_CASE("header_parser throws on version with multiple dots", "[parser][header][corrupted]")
{
    auto data = make_bytes("%PDF-1.2.3\n");
    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(),
                        Catch::Matchers::ContainsSubstring("Invalid PDF header version format"));
}

TEST_CASE("header_parser throws when reader is null", "[parser][header][corrupted]")
{
    document doc{nullptr, nullptr};
    header_parser parser{doc};

    REQUIRE_THROWS_AS(parser.parse(), io_exception);
}
} // namespace ripper::pdf::core
