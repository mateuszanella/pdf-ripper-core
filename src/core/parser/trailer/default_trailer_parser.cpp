#include "core/parser/trailer/default_trailer_parser.hpp"

#include "core/document/identifier.hpp"
#include "core/document/object/object.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/value_parsing.hpp"
#include "core/util/text.hpp"

#include <limits>
#include <string>
#include <string_view>

namespace ripper::pdf::core
{
trailer default_trailer_parser::parse_dictionary(std::string_view content)
{
    pdf_lexer lexer{content};
    dictionary dict{};

    // Scan forward until we find the opening << of the trailer dictionary.
    // Anything before it (e.g. whitespace, comments) is silently skipped.
    bool found_dictionary = false;
    while (!found_dictionary)
    {
        const auto token = lexer.next();
        if (token.type == lexer_token_type::eof)
            throw parse_exception{"Trailer dictionary was not found"};

        if (token.type == lexer_token_type::dictionary_begin)
            found_dictionary = true;
    }

    // Iteratively parse key-object pairs from the trailer dictionary.
    // We only extract the fields we care about (Size, Prev, Root, ID)
    // and skip everything else to stay resilient against unknown entries.
    while (true)
    {
        const auto token = lexer.peek();

        // >> signals the end of the dictionary, which means we are done.
        if (token.type == lexer_token_type::dictionary_end)
        {
            (void)lexer.next();
            break;
        }

        if (token.type == lexer_token_type::eof)
            throw parse_exception{"Unexpected EOF while parsing trailer"};

        // Trailer dictionary keys must be PDF names (`/Key`).
        // If we encounter something else, skip it and move on.
        if (token.type != lexer_token_type::name)
        {
            lexer.skip_value();

            continue;
        }

        const auto key_token = lexer.next();

        // /Size — total number of entries in the xref table (required).
        // /Prev — byte offset of the previous xref/trailer pair, used to
        //         walk the update chain in linearized or incrementally updated PDFs.
        if (key_token.lexeme == "Size" || key_token.lexeme == "Prev")
        {
            auto peek_result = lexer.peek();

            if (peek_result.type == lexer_token_type::integer)
            {
                const auto val = text::parse_size_t(peek_result.lexeme);
                (void)lexer.next();
                if (val)
                    // Finally, if we successfully parsed a valid integer, set the corresponding
                    // field in the dictionary.
                    dict.set(std::string{key_token.lexeme},
                             object{static_cast<std::int64_t>(*val)});
            }
            else
            {
                // Value is present but not an integer, skip it gracefully.
                lexer.skip_value();
            }

            continue;
        }

        // /Root — indirect reference to the document catalog indirect_object (required).
        // This is the entry point for the entire logical document structure.
        if (key_token.lexeme == "Root")
        {
            auto ref = parse_indirect_reference(lexer);

            dict.set("Root", object{ref});

            continue;
        }

        // /ID — optional two-element array of hex strings that uniquely identify
        // this PDF. The first string is the original ID (set when the file was
        // created) and the second reflects the current revision.
        if (key_token.lexeme == "ID")
        {
            auto begin_result = lexer.next();

            // ID must be an array, if it isn't, skip whatever object is there.
            if (begin_result.type != lexer_token_type::array_begin)
            {
                lexer.skip_value();
                continue;
            }

            // Read the first (original) ID string. If the /ID field is set,
            // the original ID string must also be set according to the spec.
            auto original_result = lexer.next();

            const bool original_is_string =
                original_result.type == lexer_token_type::hex_string ||
                original_result.type == lexer_token_type::literal_string;

            if (!original_is_string)
                throw parse_exception{"Trailer ID original object must be a string"};

            array id_array{};
            id_array.emplace_back(object{std::string{original_result.lexeme}});

            // Some PDFs only include one ID string instead of the required two,
            // so we need to check if the next token is the closing ] before
            // trying to read the second string.
            auto next_result = lexer.peek();

            if (next_result.type == lexer_token_type::array_end)
            {
                (void)lexer.next();

                dict.set("ID", object{std::move(id_array)});

                continue;
            }

            // Read the second (current revision) ID string.
            auto current_result = lexer.next();

            const bool current_is_string = current_result.type == lexer_token_type::hex_string ||
                                           current_result.type == lexer_token_type::literal_string;

            if (!current_is_string)
                throw parse_exception{"Trailer ID current object must be a string"};

            // Expect the closing ] of the ID array.
            auto end_result = lexer.next();

            if (end_result.type != lexer_token_type::array_end)
                throw parse_exception{"Trailer ID array is not properly terminated"};

            id_array.emplace_back(object{std::string{current_result.lexeme}});

            dict.set("ID", object{std::move(id_array)});

            continue;
        }

        // Unknown or unhandled key. Skip it and its object to stay resilient
        // against future PDF extensions that may introduce new trailer fields.
        lexer.skip_value();
    }

    return trailer{std::move(dict)};
}

trailer default_trailer_parser::parse(std::string_view content)
{
    // The trailer section always begins with the literal keyword "trailer",
    // followed by the dictionary. Locate it first before handing off to
    // the dictionary parser.
    const std::size_t trailerPos = content.find("trailer");
    if (trailerPos == std::string_view::npos)
    {
        throw parse_exception{"Missing trailer keyword"};
    }

    content = content.substr(trailerPos + 7); // skip "trailer"

    return parse_dictionary(content);
}
} // namespace ripper::pdf::core
