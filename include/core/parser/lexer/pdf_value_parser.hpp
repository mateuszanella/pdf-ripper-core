#pragma once

#include <expected>
#include <string_view>

#include "core/error.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"

namespace ripper::core
{
    class pdf_value_parser
    {
    public:
        // Consumes a compound object (dictionary or array), including nested.
        static std::expected<void, error>
        consume_compound_object(
            pdf_lexer &lexer,
            lexer_token_type begin_token,
            lexer_token_type end_token,
            error error_code);

        // Consumes a single value (primitive, array, dictionary, or indirect reference).
        static std::expected<void, error>
        consume_value(
            pdf_lexer &lexer,
            error error_code);

        // Parses an indirect reference (obj gen R) and returns (obj, gen).
        static std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
        parse_reference_tokens(pdf_lexer &lexer, error error_code);
    };
}
