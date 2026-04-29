#pragma once

#include <cstdint>
#include <deque>
#include <expected>
#include <limits>
#include <string_view>
#include <utility>

#include "core/error.hpp"

namespace ripper::core
{
    enum class lexer_token_type
    {
        eof,                // %%EOF
        dictionary_begin,   // <<
        dictionary_end,     // >>
        array_begin,        // [
        array_end,          // ]
        name,               // /Name
        integer,            // 123
        real,               // 123.45
        keyword,            // true, false, null
        literal_string,     // (String)
        hex_string          // <48656C6C6F>
    };

    struct lexer_token
    {
        lexer_token_type type;
        std::string_view lexeme;
    };

    /// Low-level PDF lexer that tokenizes a raw content buffer.
    ///
    /// This type operates on a `string_view` of PDF content and produces
    /// `lexer_token` values one at a time. It supports lookahead, consumption
    /// by type/lexeme, and compound structure skipping.
    ///
    /// Tokens are produced lazily and cached for lookahead via an internal deque.
    class pdf_lexer
    {
    public:
        /// Construct a lexer over the given content buffer.
        ///
        /// The lexer stores a view and does not take ownership of the content.
        /// The content must outlive the lexer.
        explicit pdf_lexer(std::string_view content);

        /// Consume and return the next token from the stream.
        ///
        /// Returns a cached lookahead token if available, otherwise reads a new one.
        /// Returns `lexer_token_type::eof` when the end of the content is reached.
        [[nodiscard]] std::expected<lexer_token, error> next();

        /// Peek at a token at the given lookahead offset without consuming it.
        ///
        /// A `lookahead` of `0` returns the next token without advancing.
        /// Tokens are buffered internally as needed.
        [[nodiscard]] std::expected<lexer_token, error> peek(std::size_t lookahead = 0);

        /// Conditionally consume the next token if it matches `type` and optionally `lexeme`.
        ///
        /// If `lexeme` is empty, only the token type is checked.
        /// Returns `true` if the token was consumed, `false` otherwise.
        bool consume(lexer_token_type type, std::string_view lexeme = {});

        /// Skip a single PDF value, including compound structures (arrays, dictionaries).
        ///
        /// If the next token is a dictionary or array begin, the entire nested
        /// structure is skipped recursively. Indirect references (`obj gen R`) are
        /// also handled.
        ///
        /// Returns `unexpected(err)` if the value cannot be skipped.
        [[nodiscard]] std::expected<void, error> skip_value();

        /// Parse an indirect reference of the form `obj gen R`.
        ///
        /// Consumes three tokens and validates their types and range constraints.
        /// Returns `unexpected(err)` if the tokens do not form a valid reference.
        [[nodiscard]] std::expected<std::pair<std::uint32_t, std::uint16_t>, error> parse_indirect_reference();

    private:
        /// Read and return the next token from the raw content buffer.
        ///
        /// Does not interact with the lookahead cache.
        [[nodiscard]] std::expected<lexer_token, error> read_token();

        /// Advance the position past all whitespace characters and `%`-style comments.
        void skip_whitespace_and_comments();

        /// Skip a compound structure (array or dictionary) given its begin/end token types.
        ///
        /// Assumes the opening token has already been consumed.
        /// Returns `unexpected(err)` if the stream ends before the structure is closed.
        [[nodiscard]] std::expected<void, error> skip_compound(lexer_token_type begin_token, lexer_token_type end_token);

        /// Returns `true` if `ch` is a PDF whitespace character or null byte.
        [[nodiscard]] static bool is_whitespace(char ch);

        /// Returns `true` if `ch` is a PDF delimiter character.
        [[nodiscard]] static bool is_delimiter(char ch);

        /// Returns `true` if `ch` can begin a numeric token (`+`, `-`, `.`, or digit).
        [[nodiscard]] static bool is_number_start(char ch);

        std::string_view _content;
        std::size_t _position{0};
        std::deque<lexer_token> _lookahead_tokens;
    };
}
