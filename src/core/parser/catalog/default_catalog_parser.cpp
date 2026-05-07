#include "core/parser/catalog/default_catalog_parser.hpp"

#include <expected>
#include <string>
#include <string_view>

#include "core/document/object/value.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/value_parsing.hpp"

namespace ripper::core
{
    std::expected<dictionary, error> default_catalog_parser::parse(std::string_view content) const
    {
        pdf_lexer lexer{content};
        dictionary dict{};

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
                auto skip_result = lexer.skip_value();
                if (!skip_result)
                    return std::unexpected(skip_result.error());

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

                dict.set("Type", value{name{"Catalog"}});

                continue;
            }

            if (key_token.lexeme == "Pages")
            {
                auto ref_result = parse_indirect_reference(lexer);
                if (!ref_result)
                    return std::unexpected(ref_result.error());

                dict.set("Pages", value{*ref_result});

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

                dict.set("Version", value{name{std::string{value_result->lexeme}}});

                continue;
            }

            // For all other keys, skip the value
            auto skip_result = lexer.skip_value();
            if (!skip_result)
                return std::unexpected(skip_result.error());
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

        return std::move(dict);
    }
}
