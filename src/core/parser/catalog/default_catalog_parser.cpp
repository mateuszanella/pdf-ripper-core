#include "core/parser/catalog/default_catalog_parser.hpp"

#include <cstddef>
#include <string_view>

#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"

namespace ripper::core
{
    namespace
    {
        parser_error to_catalog_error(lexer_error error)
        {
            switch (error)
            {
            case lexer_error::unexpected_eof:
            case lexer_error::unterminated_hex_string:
            case lexer_error::unterminated_literal_string:
                return parser_error::unexpected_eof;
            default:
                return parser_error::corrupted_catalog;
            }
        }
    }

    std::expected<indirect_reference, parser_error> default_catalog_parser::parse(
        std::string_view content,
        indirect_reference catalog_ref) const
    {
        pdf_lexer lexer{content};

        bool found_dictionary = false;
        while (!found_dictionary)
        {
            auto token_result = lexer.next();
            if (!token_result)
            {
                return std::unexpected(to_catalog_error(token_result.error()));
            }

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
            {
                break;
            }

            if (token.type == lexer_token_type::dictionary_begin)
            {
                found_dictionary = true;
            }
        }

        if (!found_dictionary)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
            {
                return std::unexpected(to_catalog_error(token_result.error()));
            }

            const auto token = *token_result;
            if (token.type == lexer_token_type::dictionary_end)
            {
                (void)lexer.next();
                break;
            }

            if (token.type == lexer_token_type::eof)
            {
                return std::unexpected(parser_error::unexpected_eof);
            }

            if (token.type != lexer_token_type::name)
            {
                auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_catalog);
                if (!consume_result)
                {
                    return std::unexpected(consume_result.error());
                }

                continue;
            }

            const auto key_token = *lexer.next();
            if (key_token.lexeme == "Type")
            {
                auto value_result = lexer.next();
                if (!value_result)
                {
                    return std::unexpected(to_catalog_error(value_result.error()));
                }

                if (value_result->type != lexer_token_type::name ||
                    value_result->lexeme != "Catalog")
                {
                    return std::unexpected(parser_error::corrupted_catalog);
                }

                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_catalog);
            if (!consume_result)
            {
                return std::unexpected(consume_result.error());
            }
        }

        return catalog_ref;
    }
}
