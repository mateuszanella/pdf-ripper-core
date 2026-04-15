#pragma once

#include <string>

namespace ripper::core
{
    /// Enumerates error codes used for programmatic branching.
    enum class error_code
    {
        // Generic, non-caller-actionable codes:
        internal_error = 0, ///< Unexpected internal failure

        invalid_argument,   ///< Invalid input provided by caller
        not_found,          ///< Requested entity was not found
        io_error,           ///< Input/output failure

        parse_error,        ///< Failure while parsing some PDF construct


        // Parsing stage specific codes:
        reader_not_open,          ///< Reader is not open
        unexpected_eof,           ///< EOF reached before expected terminator

        /// Cross-reference table related errors:
        corrupted_xref_table,     ///< XRef table is malformed and cannot be parsed
        xref_entry_not_in_use,    ///< XRef entry exists but is marked free
        xref_entry_not_found,     ///< XRef entry for requested reference not found
        generation_mismatch,      ///< Generation does not match requested reference

        /// Tokenization/parsing errors:
        offset_out_of_bounds,     ///< Requested offset is outside readable range
        tokenization_error,       ///< Lexer failed to produce a token
        invalid_object_boundary,  ///< Invalid start/end boundaries while slicing object
    };

    /// Value-semantic error object.
    ///
    /// An `error` encapsulates an `error_code` and an optional message.
    /// The code is intended for programmatic branching, while the message
    /// provides human-readable diagnostic context.
    ///
    /// This type has no success state; every instance represents a failure.
    /// It is intended to be used as the error type in `std::expected`.
    class [[nodiscard]] error
    {
    public:
        /// Create an error with code `internal_error` and empty message.
        ///
        /// This represents an unspecified failure.
        constexpr error() noexcept = default;

        /// Create an error with the specified `code`.
        ///
        /// The message is initialized to empty.
        constexpr error(error_code code) noexcept
            : code_{code} {}

        /// Create an error with the specified `code` and `message`.
        ///
        /// The message provides additional diagnostic context.
        error(error_code code, std::string message)
            : code_{code}, message_{std::move(message)} {}

        /// Return the error code.
        ///
        /// This value should be used for programmatic branching.
        [[nodiscard]] constexpr error_code code() const noexcept { return code_; }

        /// Return the diagnostic message.
        ///
        /// The message may be empty and must not be used for control flow.
        [[nodiscard]] const std::string &message() const noexcept { return message_; }

        /// Return `true` if this error has the specified `code`.
        ///
        /// This is equivalent to `this->code() == code`.
        [[nodiscard]] constexpr bool is(error_code code) const noexcept { return code_ == code; }

    private:
        error_code code_{error_code::internal_error};
        std::string message_;
    };
}
