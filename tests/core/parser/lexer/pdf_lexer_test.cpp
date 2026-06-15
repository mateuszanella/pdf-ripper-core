#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"

#include <catch2/catch_test_macros.hpp>

namespace ripper::pdf::core
{
TEST_CASE("pdf_lexer tokenizes a mixed content stream", "[parser][lexer]")
{
    pdf_lexer lexer{"<< /Type /Catalog /Count 2 /Kids [1 0 R 2 0 R] >>"};

    REQUIRE(lexer.next().type == lexer_token_type::dictionary_begin);

    const auto type_key = lexer.next();
    REQUIRE(type_key.type == lexer_token_type::name);
    REQUIRE(type_key.lexeme == "Type");

    const auto type_name = lexer.next();
    REQUIRE(type_name.type == lexer_token_type::name);
    REQUIRE(type_name.lexeme == "Catalog");

    REQUIRE(lexer.next().lexeme == "Count");

    const auto count = lexer.next();
    REQUIRE(count.type == lexer_token_type::integer);
    REQUIRE(count.lexeme == "2");

    REQUIRE(lexer.next().lexeme == "Kids");
    REQUIRE(lexer.next().type == lexer_token_type::array_begin);
    REQUIRE(lexer.next().lexeme == "1");
    REQUIRE(lexer.next().lexeme == "0");
    REQUIRE(lexer.next().lexeme == "R");
    REQUIRE(lexer.next().lexeme == "2");
    REQUIRE(lexer.next().lexeme == "0");
    REQUIRE(lexer.next().lexeme == "R");
    REQUIRE(lexer.next().type == lexer_token_type::array_end);
    REQUIRE(lexer.next().type == lexer_token_type::dictionary_end);
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer skips comments and supports lookahead/consume", "[parser][lexer]")
{
    pdf_lexer lexer{"% comment line\n123 /Name"};

    const auto peeked = lexer.peek();
    REQUIRE(peeked.type == lexer_token_type::integer);
    REQUIRE(peeked.lexeme == "123");

    REQUIRE(lexer.consume(lexer_token_type::integer, "123"));
    REQUIRE(lexer.consume(lexer_token_type::name, "Name"));
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer rejects invalid numeric lexemes", "[parser][lexer]")
{
    pdf_lexer lexer{"+"};

    REQUIRE_THROWS_AS((void)lexer.next(), parse_exception);
}
} // namespace ripper::pdf::core
