#include "core/parser/catalog/pages/default_pages_parser.hpp"

#include <expected>
#include <limits>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<parsed_pages, error>
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
                continue;
            }

            if (key_token.lexeme == "Count")
            {
                auto value_result = lexer.next();
                if (!value_result)
                    return std::unexpected(value_result.error());
                if (value_result->type != lexer_token_type::integer)
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_object)
                                               .with_component(error_component::pages)
                                               .with_field("Count")
                                               .with_expected("integer")
                                               .with_actual(std::string{value_result->lexeme})
                                               .with_message("Pages Count must be an integer")
                                               .build());
                const auto count = text::parse_size_t(value_result->lexeme);
                if (!count || *count > std::numeric_limits<std::uint32_t>::max())
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_object)
                                               .with_component(error_component::pages)
                                               .with_field("Count")
                                               .with_expected("0..4294967295")
                                               .with_actual(std::string{value_result->lexeme})
                                               .with_message("Pages Count is out of range")
                                               .build());
                out.count = static_cast<std::uint32_t>(*count);
                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(
                lexer,
                error_builder::create()
                    .with_code(error_code::corrupted_object)
                    .with_component(error_component::pages)
                    .with_field("dictionary_entry")
                    .with_expected("known key or valid value")
                    .with_message("Unexpected value in pages dictionary")
                    .build());
            if (!consume_result)
                return std::unexpected(consume_result.error());
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
        return out;
    }
}
