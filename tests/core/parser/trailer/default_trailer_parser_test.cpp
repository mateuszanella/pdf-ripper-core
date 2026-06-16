#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/trailer/default_trailer_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace ripper::pdf::core
{
TEST_CASE("default_trailer_parser parses full trailer", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
trailer
<<
/Size 8
/Root 1 0 R
/Prev 42
/ID [<0123456789ABCDEF> <FEDCBA9876543210>]
>>
)");

    REQUIRE(result.size() == 8);

    const auto root = result.root();
    REQUIRE(root.has_value());
    REQUIRE(root->object_number() == 1);
    REQUIRE(root->generation() == 0);

    const auto prev = result.prev();
    REQUIRE(prev.has_value());
    REQUIRE(*prev == 42);

    const auto id = result.id();
    REQUIRE(id.has_value());
    REQUIRE(id->original() == "0123456789ABCDEF");
    REQUIRE(id->current().has_value());
    REQUIRE(id->current().value() == "FEDCBA9876543210");
}

TEST_CASE("default_trailer_parser parses minimal trailer", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse("trailer\n<<\n/Size 5\n/Root 10 0 R\n>>\n");

    REQUIRE(result.size() == 5);

    const auto root = result.root();
    REQUIRE(root.has_value());
    REQUIRE(root->object_number() == 10);
    REQUIRE(root->generation() == 0);

    REQUIRE_FALSE(result.prev().has_value());
    REQUIRE_FALSE(result.id().has_value());
}

TEST_CASE("default_trailer_parser parses trailer with whitespace and comments", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
% some comment before trailer keyword
trailer
% comment between trailer and dict
<< % inline comment
/Size 3
/Root 1 0 R
>>
)");

    REQUIRE(result.size() == 3);

    const auto root = result.root();
    REQUIRE(root.has_value());
    REQUIRE(root->object_number() == 1);
}

TEST_CASE("default_trailer_parser parses ID with single string", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/ID [<0123456789ABCDEF>]
>>
)");

    const auto id = result.id();
    REQUIRE(id.has_value());
    REQUIRE(id->original() == "0123456789ABCDEF");
    REQUIRE_FALSE(id->current().has_value());
}

TEST_CASE("default_trailer_parser parses literal string ID", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/ID [(original) (current)]
>>
)");

    const auto id = result.id();
    REQUIRE(id.has_value());
    REQUIRE(id->original() == "original");
    REQUIRE(id->current().has_value());
    REQUIRE(id->current().value() == "current");
}

TEST_CASE("default_trailer_parser skips unknown keys", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
trailer
<<
/Unknown /SomeName
/Size 2
/AnotherUnknown 123 456 false
/Root 1 0 R
>>
)");

    REQUIRE(result.size() == 2);

    const auto root = result.root();
    REQUIRE(root.has_value());
    REQUIRE(root->object_number() == 1);
}

TEST_CASE("default_trailer_parser throws on missing trailer keyword",
          "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("<< /Size 8 /Root 1 0 R >>"),
                        Catch::Matchers::ContainsSubstring("Missing trailer keyword"));
}

TEST_CASE("default_trailer_parser throws on missing dictionary", "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("trailer /Size 8 /Root 1 0 R"),
                        Catch::Matchers::ContainsSubstring("Trailer dictionary was not found"));
}

TEST_CASE("default_trailer_parser throws on unterminated dictionary",
          "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("trailer\n<<\n/Size 8\n/Root 1 0 R\n"),
                        Catch::Matchers::ContainsSubstring("trailer"));
}

TEST_CASE("default_trailer_parser throws on invalid ID original string",
          "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(
        parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/ID [123 456]
>>
)"),
        Catch::Matchers::ContainsSubstring("Trailer ID original object must be a string"));
}

TEST_CASE("default_trailer_parser throws on invalid ID current string",
          "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(
        parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/ID [<ABC> 123]
>>
)"),
        Catch::Matchers::ContainsSubstring("Trailer ID current object must be a string"));
}

TEST_CASE("default_trailer_parser throws on unterminated ID array", "[parser][trailer][corrupted]")
{
    default_trailer_parser parser;

    REQUIRE_THROWS_WITH(
        parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/ID [<ABC> <DEF> % unterminated
>>
)"),
        Catch::Matchers::ContainsSubstring("Trailer ID array is not properly terminated"));
}

TEST_CASE("default_trailer_parser handles null Prev gracefully", "[parser][trailer]")
{
    default_trailer_parser parser;

    const auto result = parser.parse(R"(
trailer
<<
/Size 1
/Root 1 0 R
/Prev null
>>
)");

    REQUIRE_FALSE(result.prev().has_value());
}
} // namespace ripper::pdf::core
