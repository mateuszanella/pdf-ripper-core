#include "core/parser/catalog/pages/default_pages_parser.hpp"

#include <expected>
#include <limits>
#include <string_view>

#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    namespace
    {
        parser_error to_pages_error(lexer_error error)
        {
            switch (error)
            {
            case lexer_error::unexpected_eof:
            case lexer_error::unterminated_hex_string:
            case lexer_error::unterminated_literal_string:
                return parser_error::unexpected_eof;
            default:
                return parser_error::corrupted_object;
            }
        }
    }

    std::expected<parsed_pages, parser_error>
    default_pages_parser::parse(std::string_view content) const
    {
        pdf_lexer lexer{content};
        parsed_pages out{};

        bool found_dictionary = false;
        bool found_type_pages = false;

        while (!found_dictionary)
        {
            auto token_result = lexer.next();
            if (!token_result)
                return std::unexpected(to_pages_error(token_result.error()));

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
                break;

            if (token.type == lexer_token_type::dictionary_begin)
                found_dictionary = true;
        }

        if (!found_dictionary)
            return std::unexpected(parser_error::corrupted_object);

        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
                return std::unexpected(to_pages_error(token_result.error()));

            const auto token = *token_result;

            if (token.type == lexer_token_type::dictionary_end)
            {
                (void)lexer.next();
                break;
            }

            if (token.type == lexer_token_type::eof)
                return std::unexpected(parser_error::unexpected_eof);

            if (token.type != lexer_token_type::name)
            {
                auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_object);
                if (!consume_result)
                    return std::unexpected(consume_result.error());
                continue;
            }

            const auto key_token = *lexer.next();

            if (key_token.lexeme == "Type")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(to_pages_error(value_result.error()));

                if (value_result->type != lexer_token_type::name || value_result->lexeme != "Pages")
                    return std::unexpected(parser_error::corrupted_object);

                found_type_pages = true;
                continue;
            }

            if (key_token.lexeme == "Count")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(to_pages_error(value_result.error()));

                if (value_result->type != lexer_token_type::integer)
                    return std::unexpected(parser_error::corrupted_object);

                const auto count = text::parse_size_t(value_result->lexeme);
                if (!count || *count > std::numeric_limits<std::uint32_t>::max())
                    return std::unexpected(parser_error::corrupted_object);

                out.count = static_cast<std::uint32_t>(*count);
                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_object);
            if (!consume_result)
                return std::unexpected(consume_result.error());
        }

        if (!found_type_pages)
            return std::unexpected(parser_error::corrupted_object);

        return out;
    }
}
