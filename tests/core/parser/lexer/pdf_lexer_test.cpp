#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

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

TEST_CASE("pdf_lexer tokenizes real numbers", "[parser][lexer]")
{
    pdf_lexer lexer{"3.14 -0.5 +2.0"};

    auto t1 = lexer.next();
    REQUIRE(t1.type == lexer_token_type::real);
    REQUIRE(t1.lexeme == "3.14");

    auto t2 = lexer.next();
    REQUIRE(t2.type == lexer_token_type::real);
    REQUIRE(t2.lexeme == "-0.5");

    auto t3 = lexer.next();
    REQUIRE(t3.type == lexer_token_type::real);
    REQUIRE(t3.lexeme == "+2.0");

    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer tokenizes boolean and null keywords", "[parser][lexer]")
{
    pdf_lexer lexer{"true false null"};

    auto t1 = lexer.next();
    REQUIRE(t1.type == lexer_token_type::keyword);
    REQUIRE(t1.lexeme == "true");

    auto t2 = lexer.next();
    REQUIRE(t2.type == lexer_token_type::keyword);
    REQUIRE(t2.lexeme == "false");

    auto t3 = lexer.next();
    REQUIRE(t3.type == lexer_token_type::keyword);
    REQUIRE(t3.lexeme == "null");

    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer tokenizes hex strings", "[parser][lexer]")
{
    pdf_lexer lexer{"<48656C6C6F> <DEADBEEF>"};

    auto t1 = lexer.next();
    REQUIRE(t1.type == lexer_token_type::hex_string);
    REQUIRE(t1.lexeme == "48656C6C6F");

    auto t2 = lexer.next();
    REQUIRE(t2.type == lexer_token_type::hex_string);
    REQUIRE(t2.lexeme == "DEADBEEF");

    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer tokenizes literal strings", "[parser][lexer]")
{
    pdf_lexer lexer{"(Hello World) (Nested(Parens)) (Escaped\\)Done)"};

    auto t1 = lexer.next();
    REQUIRE(t1.type == lexer_token_type::literal_string);
    REQUIRE(t1.lexeme == "Hello World");

    auto t2 = lexer.next();
    REQUIRE(t2.type == lexer_token_type::literal_string);
    REQUIRE(t2.lexeme == "Nested(Parens)");

    auto t3 = lexer.next();
    REQUIRE(t3.type == lexer_token_type::literal_string);
    REQUIRE(t3.lexeme == "Escaped\\)Done");

    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer throws on unterminated hex string", "[parser][lexer][corrupted]")
{
    pdf_lexer lexer{"<48656C6C6F"};
    REQUIRE_THROWS_WITH((void)lexer.next(),
                        Catch::Matchers::ContainsSubstring("Unterminated hex string"));
}

TEST_CASE("pdf_lexer throws on unterminated literal string", "[parser][lexer][corrupted]")
{
    pdf_lexer lexer{"(Hello World"};
    REQUIRE_THROWS_WITH((void)lexer.next(),
                        Catch::Matchers::ContainsSubstring("Unterminated literal string"));
}

TEST_CASE("pdf_lexer throws on invalid dictionary end", "[parser][lexer][corrupted]")
{
    pdf_lexer lexer{">"};
    REQUIRE_THROWS_WITH((void)lexer.next(),
                        Catch::Matchers::ContainsSubstring("Invalid dictionary end token"));
}

TEST_CASE("pdf_lexer throws on unexpected characters", "[parser][lexer][corrupted]")
{
    SECTION("opening brace")
    {
        pdf_lexer lexer{"{"};
        REQUIRE_THROWS_WITH((void)lexer.next(), Catch::Matchers::ContainsSubstring(
                                                    "Unexpected character while lexing"));
    }

    SECTION("closing brace")
    {
        pdf_lexer lexer{"}"};
        REQUIRE_THROWS_WITH((void)lexer.next(), Catch::Matchers::ContainsSubstring(
                                                    "Unexpected character while lexing"));
    }

    SECTION("at sign")
    {
        pdf_lexer lexer{"@"};
        const auto tok = lexer.next();
        REQUIRE(tok.type == lexer_token_type::keyword);
        REQUIRE(tok.lexeme == "@");
    }

    SECTION("tilde")
    {
        pdf_lexer lexer{"~"};
        const auto tok = lexer.next();
        REQUIRE(tok.type == lexer_token_type::keyword);
        REQUIRE(tok.lexeme == "~");
    }
}

TEST_CASE("pdf_lexer throws on invalid numeric lexemes with messages", "[parser][lexer][corrupted]")
{
    SECTION("bare plus")
    {
        pdf_lexer lexer{"+"};
        REQUIRE_THROWS_WITH((void)lexer.next(),
                            Catch::Matchers::ContainsSubstring("Invalid numeric token lexeme"));
    }

    SECTION("bare minus")
    {
        pdf_lexer lexer{"-"};
        REQUIRE_THROWS_WITH((void)lexer.next(),
                            Catch::Matchers::ContainsSubstring("Invalid numeric token lexeme"));
    }

    SECTION("bare dot")
    {
        pdf_lexer lexer{"."};
        REQUIRE_THROWS_WITH((void)lexer.next(),
                            Catch::Matchers::ContainsSubstring("Invalid numeric token lexeme"));
    }

    SECTION("plus dot")
    {
        pdf_lexer lexer{"+."};
        REQUIRE_THROWS_WITH((void)lexer.next(),
                            Catch::Matchers::ContainsSubstring("Invalid numeric token lexeme"));
    }

    SECTION("minus dot")
    {
        pdf_lexer lexer{"-."};
        REQUIRE_THROWS_WITH((void)lexer.next(),
                            Catch::Matchers::ContainsSubstring("Invalid numeric token lexeme"));
    }
}

TEST_CASE("pdf_lexer returns eof on empty input", "[parser][lexer]")
{
    pdf_lexer lexer{""};
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer handles whitespace-only input", "[parser][lexer]")
{
    pdf_lexer lexer{"   \t\n\r  "};
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer handles comment-only input", "[parser][lexer]")
{
    pdf_lexer lexer{"% just a comment\n% another comment"};
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer tokenizes adjacent tokens without whitespace", "[parser][lexer]")
{
    pdf_lexer lexer{"[true]"};

    auto t1 = lexer.next();
    REQUIRE(t1.type == lexer_token_type::array_begin);

    auto t2 = lexer.next();
    REQUIRE(t2.type == lexer_token_type::keyword);
    REQUIRE(t2.lexeme == "true");

    REQUIRE(lexer.next().type == lexer_token_type::array_end);
    REQUIRE(lexer.next().type == lexer_token_type::eof);
}

TEST_CASE("pdf_lexer handles lookahead across multiple tokens", "[parser][lexer]")
{
    pdf_lexer lexer{"1 2 3 4 5"};

    REQUIRE(lexer.peek(0).lexeme == "1");
    REQUIRE(lexer.peek(1).lexeme == "2");
    REQUIRE(lexer.peek(2).lexeme == "3");
    REQUIRE(lexer.peek(3).lexeme == "4");
    REQUIRE(lexer.peek(4).lexeme == "5");

    REQUIRE(lexer.next().lexeme == "1");
    REQUIRE(lexer.next().lexeme == "2");
    REQUIRE(lexer.peek(0).lexeme == "3");
    REQUIRE(lexer.next().lexeme == "3");
}
} // namespace ripper::pdf::core
