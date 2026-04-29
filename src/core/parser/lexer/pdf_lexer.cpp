#include "core/parser/lexer/pdf_lexer.hpp"

#include <cctype>
#include <limits>
#include <string>

#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    namespace
    {
        bool is_invalid_number_lexeme(std::string_view lexeme)
        {
            return lexeme == "+" || lexeme == "-" || lexeme == "." || lexeme == "+." || lexeme == "-.";
        }
    }

    pdf_lexer::pdf_lexer(std::string_view content)
        : _content{content}
    {
    }

    std::expected<lexer_token, error> pdf_lexer::next()
    {
        if (!_lookahead_tokens.empty())
        {
            const auto token = _lookahead_tokens.front();
            _lookahead_tokens.pop_front();
            return token;
        }

        return read_token();
    }

    std::expected<lexer_token, error> pdf_lexer::peek(std::size_t lookahead)
    {
        while (_lookahead_tokens.size() <= lookahead)
        {
            auto token_result = read_token();
            if (!token_result)
            {
                return std::unexpected(token_result.error());
            }

            _lookahead_tokens.push_back(*token_result);
        }

        return _lookahead_tokens[lookahead];
    }

    bool pdf_lexer::consume(lexer_token_type type, std::string_view lexeme)
    {
        auto token_result = peek();
        if (!token_result)
        {
            return false;
        }

        const auto token = *token_result;
        if (token.type != type)
        {
            return false;
        }

        if (!lexeme.empty() && token.lexeme != lexeme)
        {
            return false;
        }

        (void)next();
        return true;
    }

    std::expected<lexer_token, error> pdf_lexer::read_token()
    {
        skip_whitespace_and_comments();

        if (_position >= _content.size())
        {
            return lexer_token{lexer_token_type::eof, {}};
        }

        const char ch = _content[_position];

        if (ch == '<')
        {
            if (_position + 1 < _content.size() && _content[_position + 1] == '<')
            {
                _position += 2;
                return lexer_token{lexer_token_type::dictionary_begin, "<<"};
            }

            ++_position;
            const std::size_t start = _position;

            while (_position < _content.size() && _content[_position] != '>')
            {
                ++_position;
            }

            if (_position >= _content.size())
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::unterminated_hex_string)
                                           .with_component(error_component::lexer)
                                           .with_offset(start)
                                           .with_field("hex_string")
                                           .with_expected(">")
                                           .with_message("Unterminated hex string")
                                           .build());
            }

            const std::size_t end = _position;
            ++_position; // consume '>'

            return lexer_token{lexer_token_type::hex_string, _content.substr(start, end - start)};
        }

        if (ch == '>')
        {
            if (_position + 1 < _content.size() && _content[_position + 1] == '>')
            {
                _position += 2;
                return lexer_token{lexer_token_type::dictionary_end, ">>"};
            }

            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_token)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_field("dictionary_end")
                                       .with_expected(">>")
                                       .with_actual(std::string{1, ch})
                                       .with_message("Invalid token")
                                       .build());
        }

        if (ch == '[')
        {
            ++_position;
            return lexer_token{lexer_token_type::array_begin, "["};
        }

        if (ch == ']')
        {
            ++_position;
            return lexer_token{lexer_token_type::array_end, "]"};
        }

        if (ch == '/')
        {
            ++_position;
            const std::size_t start = _position;

            while (_position < _content.size())
            {
                const char current = _content[_position];
                if (is_whitespace(current) || is_delimiter(current))
                {
                    break;
                }

                ++_position;
            }

            return lexer_token{lexer_token_type::name, _content.substr(start, _position - start)};
        }

        if (ch == '(')
        {
            ++_position;
            const std::size_t start = _position;

            std::size_t depth = 1;
            bool escaped = false;

            while (_position < _content.size())
            {
                const char current = _content[_position];
                ++_position;

                if (escaped)
                {
                    escaped = false;
                    continue;
                }

                if (current == '\\')
                {
                    escaped = true;
                    continue;
                }

                if (current == '(')
                {
                    ++depth;
                    continue;
                }

                if (current == ')')
                {
                    --depth;
                    if (depth == 0)
                    {
                        const std::size_t end = _position - 1;
                        return lexer_token{lexer_token_type::literal_string, _content.substr(start, end - start)};
                    }
                }
            }

            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unterminated_literal_string)
                                       .with_component(error_component::lexer)
                                       .with_offset(start)
                                       .with_field("literal_string")
                                       .with_expected(")")
                                       .with_message("Unterminated literal string")
                                       .build());
        }

        if (is_number_start(ch))
        {
            const std::size_t start = _position;
            bool saw_digit = std::isdigit(static_cast<unsigned char>(ch)) != 0;
            bool saw_dot = (ch == '.');

            ++_position;

            while (_position < _content.size())
            {
                const char current = _content[_position];
                if (std::isdigit(static_cast<unsigned char>(current)) != 0)
                {
                    saw_digit = true;
                    ++_position;
                    continue;
                }

                if (current == '.' && !saw_dot)
                {
                    saw_dot = true;
                    ++_position;
                    continue;
                }

                break;
            }

            const std::string_view lexeme = _content.substr(start, _position - start);
            if (!saw_digit || is_invalid_number_lexeme(lexeme))
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::invalid_token)
                                           .with_component(error_component::lexer)
                                           .with_offset(start)
                                           .with_field("numeric_token")
                                           .with_actual(std::string{lexeme})
                                           .with_message("Invalid numeric token")
                                           .build());
            }

            return lexer_token{
                saw_dot ? lexer_token_type::real : lexer_token_type::integer,
                lexeme};
        }

        const std::size_t start = _position;
        while (_position < _content.size())
        {
            const char current = _content[_position];
            if (is_whitespace(current) || is_delimiter(current))
            {
                break;
            }

            ++_position;
        }

        if (_position == start)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_token)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_field("token")
                                       .with_actual(std::string{1, ch})
                                       .with_message("Invalid token")
                                       .build());
        }

        return lexer_token{lexer_token_type::keyword, _content.substr(start, _position - start)};
    }

    void pdf_lexer::skip_whitespace_and_comments()
    {
        while (_position < _content.size())
        {
            const char ch = _content[_position];

            if (is_whitespace(ch))
            {
                ++_position;
                continue;
            }

            if (ch == '%')
            {
                ++_position;
                while (_position < _content.size())
                {
                    const char current = _content[_position];
                    if (current == '\n' || current == '\r')
                    {
                        break;
                    }

                    ++_position;
                }

                continue;
            }

            break;
        }
    }

    bool pdf_lexer::is_whitespace(char ch)
    {
        return std::isspace(static_cast<unsigned char>(ch)) != 0 || ch == '\0';
    }

    bool pdf_lexer::is_delimiter(char ch)
    {
        switch (ch)
        {
        case '(':
        case ')':
        case '<':
        case '>':
        case '[':
        case ']':
        case '{':
        case '}':
        case '/':
        case '%':
            return true;
        default:
            return false;
        }
    }

    bool pdf_lexer::is_number_start(char ch)
    {
        return std::isdigit(static_cast<unsigned char>(ch)) != 0 || ch == '+' || ch == '-' || ch == '.';
    }

    std::expected<void, error> pdf_lexer::skip_compound(
        lexer_token_type begin_token,
        lexer_token_type end_token)
    {
        std::size_t depth = 1;
        while (depth > 0)
        {
            auto token_result = next();
            if (!token_result)
                return std::unexpected(token_result.error());

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::unexpected_eof)
                                           .with_component(error_component::lexer)
                                           .with_offset(_position)
                                           .with_message("Unexpected EOF while skipping compound structure")
                                           .build());

            if (token.type == begin_token)
                ++depth;
            else if (token.type == end_token)
                --depth;
        }
        return {};
    }

    std::expected<void, error> pdf_lexer::skip_value()
    {
        auto token_result = next();
        if (!token_result)
            return std::unexpected(token_result.error());

        const auto token = *token_result;
        if (token.type == lexer_token_type::eof ||
            token.type == lexer_token_type::dictionary_end ||
            token.type == lexer_token_type::array_end)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_token)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_message("Expected a value but got an invalid or unexpected token")
                                       .build());

        if (token.type == lexer_token_type::dictionary_begin)
            return skip_compound(lexer_token_type::dictionary_begin, lexer_token_type::dictionary_end);

        if (token.type == lexer_token_type::array_begin)
            return skip_compound(lexer_token_type::array_begin, lexer_token_type::array_end);

        if (token.type == lexer_token_type::integer)
        {
            auto gen = peek();
            auto marker = peek(1);
            if (gen && marker &&
                gen->type == lexer_token_type::integer &&
                marker->type == lexer_token_type::keyword &&
                marker->lexeme == "R")
            {
                (void)next();
                (void)next();
            }
        }

        return {};
    }

    std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
    pdf_lexer::parse_indirect_reference()
    {
        auto obj = next();
        auto gen = next();
        auto marker = next();

        if (!obj || !gen || !marker)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unexpected_eof)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_message("Unexpected EOF while parsing indirect reference")
                                       .build());

        if (obj->type != lexer_token_type::integer ||
            gen->type != lexer_token_type::integer ||
            marker->type != lexer_token_type::keyword ||
            marker->lexeme != "R")
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_token)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_expected("obj gen R")
                                       .with_message("Invalid indirect reference")
                                       .build());

        const auto obj_num = text::parse_size_t(obj->lexeme);
        const auto gen_num = text::parse_size_t(gen->lexeme);

        if (!obj_num || !gen_num ||
            *obj_num > std::numeric_limits<std::uint32_t>::max() ||
            *gen_num > std::numeric_limits<std::uint16_t>::max())
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::invalid_token)
                                       .with_component(error_component::lexer)
                                       .with_offset(_position)
                                       .with_message("Indirect reference numbers are out of range")
                                       .build());

        return std::make_pair(
            static_cast<std::uint32_t>(*obj_num),
            static_cast<std::uint16_t>(*gen_num));
    }
}
