#include "core/parser/catalog/default_catalog_parser.hpp"

#include <expected>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "core/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"

namespace ripper::core
{
    std::expected<parsed_catalog, error>
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
                return std::unexpected(token_result.error());

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
                break;

            if (token.type == lexer_token_type::dictionary_begin)
                found_dictionary = true;
        }

        if (!found_dictionary)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_catalog)
                                       .with_component(error_component::catalog)
                                       .with_field("dictionary")
                                       .with_expected("<< ... >>")
                                       .with_message("Catalog dictionary was not found")
                                       .build());

        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
                return std::unexpected(token_result.error());

            const auto token = *token_result;

            if (token.type == lexer_token_type::dictionary_end)
            {
                (void)lexer.next();
                break;
            }

            if (token.type == lexer_token_type::eof)
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::unexpected_eof)
                                           .with_component(error_component::catalog)
                                           .with_field("dictionary")
                                           .with_message("Unexpected EOF while parsing catalog")
                                           .build());

            if (token.type != lexer_token_type::name)
            {
                auto consume_result = pdf_value_parser::consume_value(
                    lexer,
                    error_builder::create()
                        .with_code(error_code::corrupted_catalog)
                        .with_component(error_component::catalog)
                        .with_field("dictionary_entry")
                        .with_expected("name key")
                        .with_message("Unexpected value in catalog dictionary")
                        .build());
                if (!consume_result)
                    return std::unexpected(consume_result.error());
                continue;
            }

            const auto key_token = *lexer.next();

            if (key_token.lexeme == "Type")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(value_result.error());
                if (value_result->type != lexer_token_type::name || value_result->lexeme != "Catalog")
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_catalog)
                                               .with_component(error_component::catalog)
                                               .with_field("Type")
                                               .with_expected("Catalog")
                                               .with_actual(std::string{value_result->lexeme})
                                               .with_message("Catalog Type must be /Catalog")
                                               .build());

                found_type_catalog = true;
                continue;
            }

            if (key_token.lexeme == "Pages")
            {
                auto ref_result = pdf_value_parser::parse_reference_tokens(
                    lexer,
                    error_builder::create()
                        .with_code(error_code::corrupted_catalog)
                        .with_component(error_component::catalog)
                        .with_field("Pages")
                        .with_expected("indirect reference")
                        .with_message("Catalog Pages entry must be an indirect reference")
                        .build());
                if (!ref_result)
                    return std::unexpected(ref_result.error());

                out.pages_ref = indirect_reference{ref_result->first, ref_result->second};
                continue;
            }

            if (key_token.lexeme == "Version")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(value_result.error());
                if (value_result->type != lexer_token_type::name)
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_catalog)
                                               .with_component(error_component::catalog)
                                               .with_field("Version")
                                               .with_expected("name")
                                               .with_actual(std::string{value_result->lexeme})
                                               .with_message("Catalog Version must be a name")
                                               .build());

                out.version = std::string{value_result->lexeme};
                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(
                lexer,
                error_builder::create()
                    .with_code(error_code::corrupted_catalog)
                    .with_component(error_component::catalog)
                    .with_field("dictionary_entry")
                    .with_expected("known key or valid value")
                    .with_message("Unexpected value in catalog dictionary")
                    .build());
            if (!consume_result)
                return std::unexpected(consume_result.error());
        }

        if (!found_type_catalog)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_catalog)
                                       .with_component(error_component::catalog)
                                       .with_field("Type")
                                       .with_expected("Catalog")
                                       .with_actual("missing")
                                       .with_message("Catalog dictionary is missing /Type /Catalog")
                                       .build());

        return out;
    }
}
