#pragma once

#include <cstddef>
#include <deque>
#include <expected>
#include <string_view>

namespace ripper::core
{
    enum class lexer_error
    {
        invalid_token,
        unexpected_eof,
        unterminated_literal_string,
        unterminated_hex_string
    };

    enum class lexer_token_type
    {
        eof,                // %%EOF
        dictionary_begin,   // <<
        dictionary_end,     // >>
        array_begin,        // [
        array_end,          // ]
        name,               // /Name
        integer,            // 123
        real,               // 123.45
        keyword,            // true, false, null
        literal_string,     // (String)
        hex_string          // <48656C6C6F>
    };

    struct lexer_token
    {
        lexer_token_type type{};
        std::string_view lexeme{};
    };

    class pdf_lexer
    {
    public:
        explicit pdf_lexer(std::string_view content);

        [[nodiscard]] std::expected<lexer_token, lexer_error> next();
        [[nodiscard]] std::expected<lexer_token, lexer_error> peek(std::size_t lookahead = 0);
        [[nodiscard]] bool consume(lexer_token_type type, std::string_view lexeme = {});

    private:
        [[nodiscard]] std::expected<lexer_token, lexer_error> read_token();
        void skip_whitespace_and_comments();

        [[nodiscard]] static bool is_whitespace(char ch);
        [[nodiscard]] static bool is_delimiter(char ch);
        [[nodiscard]] static bool is_number_start(char ch);

        std::string_view _content;
        std::size_t _position{0};
        std::deque<lexer_token> _lookahead_tokens{};
    };
}
