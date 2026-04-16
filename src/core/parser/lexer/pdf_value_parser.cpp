#include "core/parser/lexer/pdf_value_parser.hpp"

#include <limits>

#include "core/error.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<void, error> pdf_value_parser::consume_compound_object(
        pdf_lexer &lexer,
        lexer_token_type begin_token,
        lexer_token_type end_token,
        error error_code)
    {
        std::size_t depth = 1;
        while (depth > 0)
        {
            auto token_result = lexer.next();
            if (!token_result)
            {
                return std::unexpected(error_code);
            }

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
            {
                return std::unexpected(error_code);
            }

            if (token.type == begin_token)
            {
                ++depth;
            }
            else if (token.type == end_token)
            {
                --depth;
            }
        }

        return {};
    }

    std::expected<void, error> pdf_value_parser::consume_value(
        pdf_lexer &lexer,
        error error_code)
    {
        auto value_result = lexer.next();
        if (!value_result)
        {
            return std::unexpected(error_code);
        }

        const auto value = *value_result;
        if (value.type == lexer_token_type::eof)
        {
            return std::unexpected(error_code);
        }

        if (value.type == lexer_token_type::dictionary_end ||
            value.type == lexer_token_type::array_end)
        {
            return std::unexpected(error_code);
        }

        if (value.type == lexer_token_type::dictionary_begin)
        {
            return consume_compound_object(
                lexer,
                lexer_token_type::dictionary_begin,
                lexer_token_type::dictionary_end,
                error_code);
        }

        if (value.type == lexer_token_type::array_begin)
        {
            return consume_compound_object(
                lexer,
                lexer_token_type::array_begin,
                lexer_token_type::array_end,
                error_code);
        }

        if (value.type == lexer_token_type::integer)
        {
            auto generation_result = lexer.peek();
            if (!generation_result)
            {
                return std::unexpected(error_code);
            }

            auto marker_result = lexer.peek(1);
            if (!marker_result)
            {
                return std::unexpected(error_code);
            }

            if (generation_result->type == lexer_token_type::integer &&
                marker_result->type == lexer_token_type::keyword &&
                marker_result->lexeme == "R")
            {
                (void)lexer.next();
                (void)lexer.next();
            }
        }

        return {};
    }

    std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
    pdf_value_parser::parse_reference_tokens(pdf_lexer &lexer, error error_code)
    {
        auto obj_token_result = lexer.next();
        auto gen_token_result = lexer.next();
        auto marker_token_result = lexer.next();

        if (!obj_token_result || !gen_token_result || !marker_token_result)
        {
            return std::unexpected(error_code);
        }

        if (obj_token_result->type != lexer_token_type::integer ||
            gen_token_result->type != lexer_token_type::integer ||
            marker_token_result->type != lexer_token_type::keyword ||
            marker_token_result->lexeme != "R")
        {
            return std::unexpected(error_code);
        }

        const auto obj_num = text::parse_size_t(obj_token_result->lexeme);
        const auto gen_num = text::parse_size_t(gen_token_result->lexeme);

        if (!obj_num || !gen_num ||
            *obj_num > std::numeric_limits<std::uint32_t>::max() ||
            *gen_num > std::numeric_limits<std::uint16_t>::max())
        {
            return std::unexpected(error_code);
        }

        return std::make_pair(
            static_cast<std::uint32_t>(*obj_num),
            static_cast<std::uint16_t>(*gen_num));
    }
}
