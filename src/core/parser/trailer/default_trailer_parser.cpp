#include "core/parser/trailer/default_trailer_parser.hpp"

#include <limits>
#include <string>
#include <string_view>

#include "core/document/identifier.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/lexer/pdf_value_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    namespace
    {
        bool is_id_string_token(const lexer_token &token)
        {
            return token.type == lexer_token_type::hex_string ||
                   token.type == lexer_token_type::literal_string;
        }
    }

    std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
    default_trailer_parser::parse_indirect_reference(std::string_view line)
    {
        pdf_lexer lexer{line};
        return pdf_value_parser::parse_reference_tokens(
            lexer,
            error_builder::create()
                .with_code(error_code::corrupted_trailer)
                .with_component(error_component::trailer)
                .with_field("indirect_reference")
                .with_expected("obj gen R")
                .with_message("Invalid trailer indirect reference")
                .build());
    }

    std::expected<trailer, error> default_trailer_parser::parse_dictionary(
        std::string_view content)
    {
        trailer::builder trailer_builder{};
        pdf_lexer lexer{content};

        bool found_dictionary = false;
        while (!found_dictionary)
        {
            auto token_result = lexer.next();
            if (!token_result)
            {
                return std::unexpected(token_result.error());
            }

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::corrupted_trailer)
                                           .with_component(error_component::trailer)
                                           .with_field("dictionary")
                                           .with_expected("<< ... >>")
                                           .with_message("Trailer dictionary was not found")
                                           .build());
            }

            if (token.type == lexer_token_type::dictionary_begin)
            {
                found_dictionary = true;
            }
        }

        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
            {
                return std::unexpected(token_result.error());
            }

            const auto token = *token_result;
            if (token.type == lexer_token_type::dictionary_end)
            {
                (void)lexer.next();
                break;
            }

            if (token.type == lexer_token_type::eof)
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::unexpected_eof)
                                           .with_component(error_component::trailer)
                                           .with_field("dictionary")
                                           .with_message("Unexpected EOF while parsing trailer")
                                           .build());
            }

            if (token.type != lexer_token_type::name)
            {
                auto consume_result = pdf_value_parser::consume_value(
                    lexer,
                    error_builder::create()
                        .with_code(error_code::corrupted_trailer)
                        .with_component(error_component::trailer)
                        .with_field("dictionary_entry")
                        .with_expected("name key")
                        .with_message("Unexpected value in trailer dictionary")
                        .build());
                if (!consume_result)
                {
                    return std::unexpected(consume_result.error());
                }

                continue;
            }

            const auto key_token = *lexer.next();

            if (key_token.lexeme == "Size")
            {
                auto value_result = lexer.peek();
                if (!value_result)
                {
                    return std::unexpected(value_result.error());
                }

                if (value_result->type == lexer_token_type::integer)
                {
                    const auto size = text::parse_size_t(value_result->lexeme);
                    (void)lexer.next();

                    if (size)
                    {
                        trailer_builder.size = static_cast<std::uint64_t>(*size);
                    }
                }
                else
                {
                    auto consume_result = pdf_value_parser::consume_value(
                        lexer,
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("Size")
                            .with_expected("integer")
                            .with_actual(std::string{value_result->lexeme})
                            .with_message("Trailer Size must be an integer")
                            .build());
                    if (!consume_result)
                    {
                        return std::unexpected(consume_result.error());
                    }
                }

                continue;
            }

            if (key_token.lexeme == "Prev")
            {
                auto value_result = lexer.peek();
                if (!value_result)
                {
                    return std::unexpected(value_result.error());
                }

                if (value_result->type == lexer_token_type::integer)
                {
                    const auto prev = text::parse_size_t(value_result->lexeme);
                    (void)lexer.next();

                    if (prev)
                    {
                        trailer_builder.prev = static_cast<std::uint64_t>(*prev);
                    }
                }
                else
                {
                    auto consume_result = pdf_value_parser::consume_value(
                        lexer,
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("Prev")
                            .with_expected("integer")
                            .with_actual(std::string{value_result->lexeme})
                            .with_message("Trailer Prev must be an integer")
                            .build());
                    if (!consume_result)
                    {
                        return std::unexpected(consume_result.error());
                    }
                }

                continue;
            }

            if (key_token.lexeme == "Root")
            {
                auto obj_token_result = lexer.peek();
                auto gen_token_result = lexer.peek(1);
                auto marker_token_result = lexer.peek(2);

                if (!obj_token_result || !gen_token_result || !marker_token_result)
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_trailer)
                                               .with_component(error_component::trailer)
                                               .with_field("Root")
                                               .with_expected("obj gen R")
                                               .with_message("Invalid Root reference in trailer")
                                               .build());
                }

                if (obj_token_result->type == lexer_token_type::integer &&
                    gen_token_result->type == lexer_token_type::integer &&
                    marker_token_result->type == lexer_token_type::keyword &&
                    marker_token_result->lexeme == "R")
                {
                    const auto obj_num = text::parse_size_t(obj_token_result->lexeme);
                    const auto gen_num = text::parse_size_t(gen_token_result->lexeme);
                    (void)lexer.next();
                    (void)lexer.next();
                    (void)lexer.next();

                    if (obj_num && gen_num &&
                        *obj_num <= std::numeric_limits<std::uint32_t>::max() &&
                        *gen_num <= std::numeric_limits<std::uint16_t>::max())
                    {
                        trailer_builder.root = indirect_reference{
                            static_cast<std::uint32_t>(*obj_num),
                            static_cast<std::uint16_t>(*gen_num)};
                    }
                }
                else
                {
                    auto consume_result = pdf_value_parser::consume_value(
                        lexer,
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("Root")
                            .with_expected("indirect reference")
                            .with_message("Trailer Root must be an indirect reference")
                            .build());
                    if (!consume_result)
                    {
                        return std::unexpected(consume_result.error());
                    }
                }

                continue;
            }

            if (key_token.lexeme == "ID")
            {
                auto begin_result = lexer.next();
                if (!begin_result)
                {
                    return std::unexpected(begin_result.error());
                }

                if (begin_result->type != lexer_token_type::array_begin)
                {
                    auto consume_result = pdf_value_parser::consume_value(
                        lexer,
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("ID")
                            .with_expected("array")
                            .with_actual(std::string{begin_result->lexeme})
                            .with_message("Trailer ID must be an array")
                            .build());
                    if (!consume_result)
                    {
                        return std::unexpected(consume_result.error());
                    }

                    continue;
                }

                auto original_result = lexer.next();
                if (!original_result)
                {
                    return std::unexpected(original_result.error());
                }

                if (!is_id_string_token(*original_result))
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_trailer)
                                               .with_component(error_component::trailer)
                                               .with_field("ID[0]")
                                               .with_expected("string")
                                               .with_actual(std::string{original_result->lexeme})
                                               .with_message("Trailer ID original value must be a string")
                                               .build());
                }

                std::string original{original_result->lexeme};

                auto next_result = lexer.peek();
                if (!next_result)
                {
                    return std::unexpected(next_result.error());
                }

                if (next_result->type == lexer_token_type::array_end)
                {
                    (void)lexer.next();
                    trailer_builder.id = identifier{std::move(original), {}};

                    continue;
                }

                auto current_result = lexer.next();
                if (!current_result)
                {
                    return std::unexpected(current_result.error());
                }

                if (!is_id_string_token(*current_result))
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_trailer)
                                               .with_component(error_component::trailer)
                                               .with_field("ID[1]")
                                               .with_expected("string")
                                               .with_actual(std::string{current_result->lexeme})
                                               .with_message("Trailer ID current value must be a string")
                                               .build());
                }

                auto end_result = lexer.next();
                if (!end_result)
                {
                    return std::unexpected(end_result.error());
                }

                if (end_result->type != lexer_token_type::array_end)
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_trailer)
                                               .with_component(error_component::trailer)
                                               .with_field("ID")
                                               .with_expected("closing ]")
                                               .with_actual(std::string{end_result->lexeme})
                                               .with_message("Trailer ID array is not properly terminated")
                                               .build());
                }

                identifier id{std::move(original), std::string{current_result->lexeme}};
                trailer_builder.id = std::move(id);

                continue;
            }

            auto consume_result = pdf_value_parser::consume_value(
                lexer,
                error_builder::create()
                    .with_code(error_code::corrupted_trailer)
                    .with_component(error_component::trailer)
                    .with_field("dictionary_entry")
                    .with_expected("known key or valid value")
                    .with_message("Unexpected value in trailer dictionary")
                    .build());
            if (!consume_result)
            {
                return std::unexpected(consume_result.error());
            }
        }

        return trailer_builder.build();
    }

    std::expected<trailer, error> default_trailer_parser::parse(std::string_view content)
    {
        const std::size_t trailerPos = content.find("trailer");
        if (trailerPos == std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_trailer)
                                       .with_component(error_component::trailer)
                                       .with_field("trailer_keyword")
                                       .with_expected("trailer")
                                       .with_message("Missing trailer keyword")
                                       .build());
        }

        content = content.substr(trailerPos + 7); // skip "trailer"
        return parse_dictionary(content);
    }
}
