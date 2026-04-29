#include "core/parser/trailer/default_trailer_parser.hpp"

#include <limits>
#include <string>
#include <string_view>

#include "core/document/identifier.hpp"
#include "core/document/object/dictionary.hpp"
#include "core/document/object/value.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
    default_trailer_parser::parse_indirect_reference(std::string_view line)
    {
        pdf_lexer lexer{line};

        return lexer.parse_indirect_reference();
    }

    std::expected<trailer, error> default_trailer_parser::parse_dictionary(std::string_view content)
    {
        pdf_lexer lexer{content};
        dictionary dict{};

        // Scan forward until we find the opening << of the trailer dictionary.
        // Anything before it (e.g. whitespace, comments) is silently skipped.
        bool found_dictionary = false;
        while (!found_dictionary)
        {
            auto token_result = lexer.next();
            if (!token_result)
                return std::unexpected(token_result.error());

            const auto token = *token_result;
            if (token.type == lexer_token_type::eof)
                return std::unexpected(
                    error_builder::create()
                        .with_code(error_code::corrupted_trailer)
                        .with_component(error_component::trailer)
                        .with_field("dictionary")
                        .with_expected("<< ... >>")
                        .with_message("Trailer dictionary was not found")
                        .build());

            if (token.type == lexer_token_type::dictionary_begin)
                found_dictionary = true;
        }

        // Iteratively parse key-value pairs from the trailer dictionary.
        // We only extract the fields we care about (Size, Prev, Root, ID)
        // and skip everything else to stay resilient against unknown entries.
        while (true)
        {
            auto token_result = lexer.peek();
            if (!token_result)
                return std::unexpected(token_result.error());

            const auto token = *token_result;

            // >> signals the end of the dictionary, which means we are done.
            if (token.type == lexer_token_type::dictionary_end)
            {
                (void)lexer.next();
                break;
            }

            if (token.type == lexer_token_type::eof)
                return std::unexpected(
                    error_builder::create()
                        .with_code(error_code::unexpected_eof)
                        .with_component(error_component::trailer)
                        .with_field("dictionary")
                        .with_message("Unexpected EOF while parsing trailer")
                        .build());

            // Trailer dictionary keys must be PDF names (`/Key`).
            // If we encounter something else, skip it and move on.
            if (token.type != lexer_token_type::name)
            {
                auto r = lexer.skip_value();
                if (!r)
                    return std::unexpected(r.error());

                continue;
            }

            const auto key_token = *lexer.next();

            // /Size — total number of entries in the xref table (required).
            // /Prev — byte offset of the previous xref/trailer pair, used to
            //         walk the update chain in linearized or incrementally updated PDFs.
            if (key_token.lexeme == "Size" || key_token.lexeme == "Prev")
            {
                auto peek_result = lexer.peek();
                if (!peek_result)
                    return std::unexpected(peek_result.error());

                if (peek_result->type == lexer_token_type::integer)
                {
                    const auto val = text::parse_size_t(peek_result->lexeme);
                    (void)lexer.next();
                    if (val)
                        // Finally, if we successfully parsed a valid integer, set the corresponding field in the dictionary.
                        dict.set(std::string{key_token.lexeme}, value{static_cast<std::int64_t>(*val)});
                }
                else
                {
                    // Value is present but not an integer, skip it gracefully.
                    auto r = lexer.skip_value();
                    if (!r)
                        return std::unexpected(r.error());
                }

                continue;
            }

            // /Root — indirect reference to the document catalog object (required).
            // This is the entry point for the entire logical document structure.
            if (key_token.lexeme == "Root")
            {
                auto ref = lexer.parse_indirect_reference();
                if (!ref)
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::corrupted_trailer)
                                               .with_component(error_component::trailer)
                                               .with_field("Root")
                                               .with_expected("obj gen R")
                                               .with_message("Root must be an indirect reference")
                                               .build());

                dict.set("Root", value{indirect_reference{ref->first, ref->second}});

                continue;
            }

            // /ID — optional two-element array of hex strings that uniquely identify
            // this PDF. The first string is the original ID (set when the file was
            // created) and the second reflects the current revision.
            if (key_token.lexeme == "ID")
            {
                auto begin_result = lexer.next();
                if (!begin_result)
                    return std::unexpected(begin_result.error());

                // ID must be an array, if it isn't, skip whatever value is there.
                if (begin_result->type != lexer_token_type::array_begin)
                {
                    auto r = lexer.skip_value();
                    if (!r)
                        return std::unexpected(r.error());
                    continue;
                }

                // Read the first (original) ID string. If the /ID field is set,
                // the original ID string must also be set according to the spec.
                auto original_result = lexer.next();
                if (!original_result)
                    return std::unexpected(original_result.error());

                const bool original_is_string =
                    original_result->type == lexer_token_type::hex_string ||
                    original_result->type == lexer_token_type::literal_string;

                if (!original_is_string)
                    return std::unexpected(
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("ID[0]")
                            .with_expected("string")
                            .with_message("Trailer ID original value must be a string")
                            .build());

                array id_array{};
                id_array.emplace_back(value{std::string{original_result->lexeme}});

                // Some PDFs only include one ID string instead of the required two,
                // so we need to check if the next token is the closing ] before
                // trying to read the second string.
                auto next_result = lexer.peek();
                if (!next_result)
                    return std::unexpected(next_result.error());

                if (next_result->type == lexer_token_type::array_end)
                {
                    (void)lexer.next();

                    dict.set("ID", value{std::move(id_array)});

                    continue;
                }

                // Read the second (current revision) ID string.
                auto current_result = lexer.next();
                if (!current_result)
                    return std::unexpected(current_result.error());

                const bool current_is_string =
                    current_result->type == lexer_token_type::hex_string ||
                    current_result->type == lexer_token_type::literal_string;

                if (!current_is_string)
                    return std::unexpected(
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("ID[1]")
                            .with_expected("string")
                            .with_message("Trailer ID current value must be a string")
                            .build());

                // Expect the closing ] of the ID array.
                auto end_result = lexer.next();
                if (!end_result)
                    return std::unexpected(end_result.error());

                if (end_result->type != lexer_token_type::array_end)
                    return std::unexpected(
                        error_builder::create()
                            .with_code(error_code::corrupted_trailer)
                            .with_component(error_component::trailer)
                            .with_field("ID")
                            .with_expected("closing ]")
                            .with_message("Trailer ID array is not properly terminated")
                            .build());

                id_array.emplace_back(value{std::string{current_result->lexeme}});

                dict.set("ID", value{std::move(id_array)});

                continue;
            }

            // Unknown or unhandled key. Skip it and its value to stay resilient
            // against future PDF extensions that may introduce new trailer fields.
            auto r = lexer.skip_value();
            if (!r)
                return std::unexpected(r.error());
        }

        return trailer{std::move(dict)};
    }

    std::expected<trailer, error> default_trailer_parser::parse(std::string_view content)
    {
        // The trailer section always begins with the literal keyword "trailer",
        // followed by the dictionary. Locate it first before handing off to
        // the dictionary parser.
        const std::size_t trailerPos = content.find("trailer");
        if (trailerPos == std::string_view::npos)
        {
            return std::unexpected(
                error_builder::create()
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
