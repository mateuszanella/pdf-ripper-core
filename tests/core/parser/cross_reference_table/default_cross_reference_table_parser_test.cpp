#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace ripper::pdf::core
{
TEST_CASE("default_cross_reference_table_parser parses single subsection", "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse(R"(xref
0 2
0000000001 00000 n
0000000002 00001 f
trailer
<< /Size 3 >>
)");

    REQUIRE(section.size() == 2);

    auto* entry0 = section.find(0);
    REQUIRE(entry0 != nullptr);
    REQUIRE(entry0->reference().object_number() == 0);
    REQUIRE(entry0->reference().generation() == 0);
    REQUIRE(entry0->offset().has_value());
    REQUIRE(*entry0->offset() == 1);
    REQUIRE(entry0->in_use());

    auto* entry1 = section.find(1);
    REQUIRE(entry1 != nullptr);
    REQUIRE(entry1->reference().object_number() == 1);
    REQUIRE(entry1->reference().generation() == 1);
    REQUIRE_FALSE(entry1->in_use());
}

TEST_CASE("default_cross_reference_table_parser parses entry at non-zero start", "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse(R"(xref
5 1
0000000042 00000 n
trailer
<< >>
)");

    REQUIRE(section.size() == 1);

    auto* entry = section.find(5);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->reference().object_number() == 5);
    REQUIRE(entry->reference().generation() == 0);
    REQUIRE(entry->offset().has_value());
    REQUIRE(*entry->offset() == 42);
    REQUIRE(entry->in_use());
}

TEST_CASE("default_cross_reference_table_parser parses entries with leading whitespace",
          "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse(R"(xref
0 1
  0000000000 00000 n
trailer
<< >>
)");

    REQUIRE(section.size() == 1);

    auto* entry = section.find(0);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->in_use());
}

TEST_CASE("default_cross_reference_table_parser parses free entry", "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse(R"(xref
0 1
0000000000 65535 f
trailer
<< >>
)");

    REQUIRE(section.size() == 1);

    auto* entry = section.find(0);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->reference().generation() == 65535);
    REQUIRE_FALSE(entry->in_use());
}

TEST_CASE("default_cross_reference_table_parser throws on missing xref keyword",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("trailer\n<< >>"),
                        Catch::Matchers::ContainsSubstring("Missing xref keyword"));
}

TEST_CASE("default_cross_reference_table_parser throws on missing subsection header",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\nno_newline"),
                        Catch::Matchers::ContainsSubstring("Unexpected EOF while parsing"));
}

TEST_CASE("default_cross_reference_table_parser throws on invalid subsection header",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\nno_space_here\nmore"),
                        Catch::Matchers::ContainsSubstring("Invalid xref subsection header"));
}

TEST_CASE("default_cross_reference_table_parser throws on non-numeric subsection range",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\nabc def\nmore"),
                        Catch::Matchers::ContainsSubstring("Invalid xref subsection range"));
}

TEST_CASE("default_cross_reference_table_parser throws on malformed entry",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\n0 1\ntoo_short\nmore"),
                        Catch::Matchers::ContainsSubstring("Malformed xref entry"));
}

TEST_CASE("default_cross_reference_table_parser throws on invalid offset",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\n0 1\nXXXXXXXXXX 00000 n\nmore"),
                        Catch::Matchers::ContainsSubstring("Invalid xref entry offset"));
}

TEST_CASE("default_cross_reference_table_parser throws on invalid generation",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\n0 1\n0000000001 XXXXX n\nmore"),
                        Catch::Matchers::ContainsSubstring("Invalid xref entry generation"));
}

TEST_CASE("default_cross_reference_table_parser throws on invalid flag",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\n0 1\n0000000001 00000 x\nmore"),
                        Catch::Matchers::ContainsSubstring("Invalid xref in-use flag"));
}

TEST_CASE("default_cross_reference_table_parser throws on EOF while parsing entries",
          "[parser][xref][corrupted]")
{
    default_cross_reference_table_parser parser;

    REQUIRE_THROWS_WITH(parser.parse("xref\n0 5\n0000000001 00000 n\n"),
                        Catch::Matchers::ContainsSubstring("Unexpected EOF while parsing"));
}

TEST_CASE("default_cross_reference_table_parser stops at trailer keyword", "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse(R"(xref
0 1
0000000001 00000 n
trailer
<< >>
)");

    REQUIRE(section.size() == 1);
}

TEST_CASE("default_cross_reference_table_parser parses empty content after xref", "[parser][xref]")
{
    default_cross_reference_table_parser parser;

    const auto section = parser.parse("xref\n0 0\ntrailer\n<< >>\n");

    REQUIRE(section.size() == 0);
}
} // namespace ripper::pdf::core
