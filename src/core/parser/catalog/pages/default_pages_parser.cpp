#include "core/parser/catalog/pages/default_pages_parser.hpp"

#include <expected>
#include <string>
#include <string_view>

#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/dictionary.hpp"
#include "core/document/object/value.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<pages, error>
    default_pages_parser::parse(std::string_view content) const
    {
        pdf_lexer lexer{content};
        dictionary dict{};

        bool found_dictionary = false;
        bool found_type_pages = false;

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
                                       .with_code(error_code::corrupted_object)
                                       .with_component(error_component::pages)
                                       .with_field("dictionary")
                                       .with_expected("<< ... >>")
                                       .with_message("Pages dictionary was not found")
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
                                           .with_component(error_component::pages)
                                           .with_field("dictionary")
                                           .with_message("Unexpected EOF while parsing pages dictionary")
                                           .build());

            if (token.type != lexer_token_type::name)
            {
                auto consume_result = pdf_value_parser::consume_value(
                    lexer,
                    error_builder::create()
                        .with_code(error_code::corrupted_object)
                        .with_component(error_component::pages)
                        .with_field("dictionary_entry")
                        .with_expected("name key")
                        .with_message("Unexpected value in pages dictionary")
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
                if (value_result->type != lexer_token_type::name || value_result->lexeme != "Pages")
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_object)
                                               .with_component(error_component::pages)
                                               .with_field("Type")
                                               .with_expected("Pages")
                                               .with_actual(std::string{value_result->lexeme})
                                               .with_message("Pages Type must be /Pages")
                                               .build());
                found_type_pages = true;
                dict.set("Type", value{name{"Pages"}});
                continue;
            }

            // For all other keys (Count, Kids, Parent, etc.), parse and store generically
            auto val_result = pdf_value_parser::parse_value(lexer);
            if (!val_result)
                return std::unexpected(val_result.error());
            dict.set(std::string{key_token.lexeme}, std::move(*val_result));
        }

        if (!found_type_pages)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_object)
                                       .with_component(error_component::pages)
                                       .with_field("Type")
                                       .with_expected("Pages")
                                       .with_actual("missing")
                                       .with_message("Pages dictionary is missing /Type /Pages")
                                       .build());

        return pages{object{std::move(dict)}};
    }
}
