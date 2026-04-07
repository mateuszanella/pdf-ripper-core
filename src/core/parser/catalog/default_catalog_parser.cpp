#include "core/parser/catalog/default_catalog_parser.hpp"

#include <expected>
#include <string>
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

    std::expected<parsed_catalog, parser_error>
    default_catalog_parser::parse(std::string_view content) const
    {
        pdf_lexer lexer{content};
        parsed_catalog out{};

        bool found_dictionary = false;
        bool found_type_catalog = false;

        while (!found_dictionary)
        {
            auto token_result = lexer.next();
            if (!token_result)
                return std::unexpected(to_catalog_error(token_result.error()));

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
                break;

            if (token.type == lexer_token_type::dictionary_begin)
                found_dictionary = true;
        }

        if (!found_dictionary)
            return std::unexpected(parser_error::corrupted_catalog);

        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
                return std::unexpected(to_catalog_error(token_result.error()));

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
                auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_catalog);
                if (!consume_result)
                    return std::unexpected(consume_result.error());
                continue;
            }

            const auto key_token = *lexer.next();

            if (key_token.lexeme == "Type")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(to_catalog_error(value_result.error()));

                if (value_result->type != lexer_token_type::name || value_result->lexeme != "Catalog")
                    return std::unexpected(parser_error::corrupted_catalog);

                found_type_catalog = true;
                continue;
            }

            if (key_token.lexeme == "Pages")
            {
                auto ref_result = pdf_value_parser::parse_reference_tokens(lexer, parser_error::corrupted_catalog);
                if (!ref_result)
                    return std::unexpected(ref_result.error());

                out.pages_ref = indirect_reference{ref_result->first, ref_result->second};
                continue;
            }

            if (key_token.lexeme == "Version")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(to_catalog_error(value_result.error()));

                if (value_result->type != lexer_token_type::name)
                    return std::unexpected(parser_error::corrupted_catalog);

                out.version = std::string{value_result->lexeme};
                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(lexer, parser_error::corrupted_catalog);
            if (!consume_result)
                return std::unexpected(consume_result.error());
        }

        if (!found_type_catalog)
            return std::unexpected(parser_error::corrupted_catalog);

        return out;
    }
}
