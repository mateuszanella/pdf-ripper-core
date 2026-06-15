#include "ripper/pdf/core/parser/value_parsing.hpp"

#include <catch2/catch_test_macros.hpp>

namespace ripper::pdf::core
{
TEST_CASE("parse_indirect_reference parses object and generation", "[parser][value]")
{
    pdf_lexer lexer{"42 7 R"};

    const auto ref = parse_indirect_reference(lexer);

    REQUIRE(ref.object_number() == 42);
    REQUIRE(ref.generation() == 7);
}

TEST_CASE("parse_value detects indirect references", "[parser][value]")
{
    pdf_lexer lexer{"15 0 R"};

    const auto value = parse_value(lexer);

    REQUIRE(value.is_indirect_reference());
    REQUIRE(value.as_indirect_reference()->object_number() == 15);
    REQUIRE(value.as_indirect_reference()->generation() == 0);
}

TEST_CASE("parse_value parses nested dictionaries and arrays", "[parser][value]")
{
    pdf_lexer lexer{"<< /Count 2 /Kids [1 0 R 2 0 R] /Type /Pages >>"};

    const auto value = parse_value(lexer);

    REQUIRE(value.is_dictionary());
    const auto* dict = value.as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* count = dict->get_integer("Count");
    REQUIRE(count != nullptr);
    REQUIRE(*count == 2);

    const auto* kids = dict->get_array("Kids");
    REQUIRE(kids != nullptr);
    REQUIRE(kids->size() == 2);
    REQUIRE((*kids)[0].is_indirect_reference());
    REQUIRE((*kids)[1].is_indirect_reference());

    const auto* type = dict->get_name("Type");
    REQUIRE(type != nullptr);
    REQUIRE(type->value == "Pages");
}

TEST_CASE("parse_indirect_reference validates malformed values", "[parser][value]")
{
    pdf_lexer malformed{"7 R"};
    REQUIRE_THROWS_AS((void)parse_indirect_reference(malformed), parse_exception);

    pdf_lexer out_of_range{"4294967296 0 R"};
    REQUIRE_THROWS_AS((void)parse_indirect_reference(out_of_range), parse_exception);
}
} // namespace ripper::pdf::core
