#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ripper::core
{
    /// Type-safe wrapper around error code enum with member methods.
    ///
    /// Wraps an underlying enum to provide encapsulated string conversion
    /// and default message generation while supporting direct enum semantics.
    class error_code
    {
    public:
        /// Underlying enum values for programmatic branching.
        enum Value : std::uint8_t
        {
            // Generic, non-caller-actionable codes:
            internal_error = 0, ///< Unexpected internal failure

            invalid_argument, ///< Invalid input provided by caller
            not_found,        ///< Requested entity was not found
            io_error,         ///< Input/output failure

            parse_error, ///< Failure while parsing some PDF construct

            // Detailed parse/domain errors for caller branching:
            missing_header,       ///< PDF header not found at expected location
            corrupted_header,     ///< PDF header found but malformed
            missing_xref_table,   ///< No cross-reference table found in document structure
            corrupted_xref_table, ///< Cross-reference table found but malformed
            missing_trailer,      ///< Trailer not found in document structure
            corrupted_trailer,    ///< Trailer found but malformed
            missing_catalog,      ///< Catalog not found in document structure
            corrupted_catalog,    ///< Catalog found but malformed
            corrupted_object,     ///< Object found but malformed
            object_not_found,     ///< Object not found

            // Detailed tokenization errors:
            invalid_token,               ///< Invalid token encountered
            unterminated_literal_string, ///< Literal string not properly terminated
            unterminated_hex_string,     ///< Hex string not properly terminated

            // Compression errors:
            compression_invalid_input,    ///< Input data is not valid for compression
            compression_buffer_too_small, ///< Compression buffer is too small
            compression_failed,           ///< Compression operation failed
            decompression_failed,         ///< Decompression operation failed
            corrupted_compressed_data,    ///< Compressed data is corrupted
            compression_memory_error,     ///< Memory error during compression

            // Parsing stage specific codes:
            reader_not_open, ///< Reader is not open
            unexpected_eof,  ///< EOF reached before expected terminator

            // Cross-reference table related errors:
            xref_entry_not_in_use, ///< XRef entry exists but is marked free
            xref_entry_not_found,  ///< XRef entry for requested reference not found
            generation_mismatch,   ///< Generation does not match requested reference

            // Tokenization/parsing errors:
            offset_out_of_bounds,    ///< Requested offset is outside readable range
            tokenization_error,      ///< Lexer failed to produce a token
            invalid_object_boundary, ///< Invalid start/end boundaries while slicing object
        };

        /// Default constructor initializes to `internal_error`.
        constexpr error_code() = default;

        /// Construct from an underlying enum value.
        explicit constexpr error_code(Value code) noexcept : value(code) {}

        /// Assign from an underlying enum value.
        constexpr error_code &operator=(Value code) noexcept
        {
            value = code;
            return *this;
        }

        /// Allow use in switch statements and direct enum comparison.
        constexpr operator Value() const noexcept { return value; }

        /// Prevent implicit boolean conversion.
        explicit operator bool() const = delete;

        /// Allow equality comparison with other error_code instances.
        constexpr bool operator==(error_code other) const noexcept { return value == other.value; }

        /// Allow inequality comparison with other error_code instances.
        constexpr bool operator!=(error_code other) const noexcept { return value != other.value; }

        /// Allow direct comparison with underlying enum values for convenience.
        constexpr bool operator==(Value other) const noexcept { return value == other; }

        /// Allow direct inequality comparison with underlying enum values for convenience.
        constexpr bool operator!=(Value other) const noexcept { return value != other; }

        /// Return a string representation of this error code.
        [[nodiscard]] constexpr std::string_view to_string() const noexcept
        {
            switch (value)
            {
                case internal_error:
                    return "internal_error";
                case invalid_argument:
                    return "invalid_argument";
                case not_found:
                    return "not_found";
                case io_error:
                    return "io_error";
                case parse_error:
                    return "parse_error";
                case missing_header:
                    return "missing_header";
                case corrupted_header:
                    return "corrupted_header";
                case missing_xref_table:
                    return "missing_xref_table";
                case corrupted_xref_table:
                    return "corrupted_xref_table";
                case missing_trailer:
                    return "missing_trailer";
                case corrupted_trailer:
                    return "corrupted_trailer";
                case missing_catalog:
                    return "missing_catalog";
                case corrupted_catalog:
                    return "corrupted_catalog";
                case corrupted_object:
                    return "corrupted_object";
                case object_not_found:
                    return "object_not_found";
                case invalid_token:
                    return "invalid_token";
                case unterminated_literal_string:
                    return "unterminated_literal_string";
                case unterminated_hex_string:
                    return "unterminated_hex_string";
                case compression_invalid_input:
                    return "compression_invalid_input";
                case compression_buffer_too_small:
                    return "compression_buffer_too_small";
                case compression_failed:
                    return "compression_failed";
                case decompression_failed:
                    return "decompression_failed";
                case corrupted_compressed_data:
                    return "corrupted_compressed_data";
                case compression_memory_error:
                    return "compression_memory_error";
                case reader_not_open:
                    return "reader_not_open";
                case unexpected_eof:
                    return "unexpected_eof";
                case xref_entry_not_in_use:
                    return "xref_entry_not_in_use";
                case xref_entry_not_found:
                    return "xref_entry_not_found";
                case generation_mismatch:
                    return "generation_mismatch";
                case offset_out_of_bounds:
                    return "offset_out_of_bounds";
                case tokenization_error:
                    return "tokenization_error";
                case invalid_object_boundary:
                    return "invalid_object_boundary";
            }

            return "internal_error";
        }

        /// Return a human-readable default message for this error code.
        [[nodiscard]] std::string default_message() const
        {
            switch (value)
            {
                case internal_error:
                    return "Unexpected internal failure";
                case invalid_argument:
                    return "Invalid input provided by caller";
                case not_found:
                    return "Requested entity was not found";
                case io_error:
                    return "Input/output failure";
                case parse_error:
                    return "Failure while parsing a PDF construct";
                case missing_header:
                    return "PDF header was not found at expected location";
                case corrupted_header:
                    return "PDF header is malformed";
                case missing_xref_table:
                    return "Cross-reference table was not found in the document structure";
                case corrupted_xref_table:
                    return "Cross-reference table is malformed";
                case missing_trailer:
                    return "Trailer was not found in the document structure";
                case corrupted_trailer:
                    return "Trailer is malformed";
                case missing_catalog:
                    return "Catalog was not found in the document structure";
                case corrupted_catalog:
                    return "Catalog is malformed";
                case corrupted_object:
                    return "Object is malformed";
                case object_not_found:
                    return "Object was not found";
                case invalid_token:
                    return "Invalid token encountered";
                case unterminated_literal_string:
                    return "Literal string is not properly terminated";
                case unterminated_hex_string:
                    return "Hex string is not properly terminated";
                case compression_invalid_input:
                    return "Input data is not valid for compression";
                case compression_buffer_too_small:
                    return "Compression buffer is too small";
                case compression_failed:
                    return "Compression operation failed";
                case decompression_failed:
                    return "Decompression operation failed";
                case corrupted_compressed_data:
                    return "Compressed data is corrupted";
                case compression_memory_error:
                    return "Memory error occurred during compression";
                case reader_not_open:
                    return "Reader is not open";
                case unexpected_eof:
                    return "EOF reached before expected terminator";
                case xref_entry_not_in_use:
                    return "Cross-reference entry exists but is marked as free";
                case xref_entry_not_found:
                    return "Cross-reference entry for requested reference was not found";
                case generation_mismatch:
                    return "Generation does not match requested reference";
                case offset_out_of_bounds:
                    return "Requested offset is outside readable range";
                case tokenization_error:
                    return "Lexer failed to produce a token";
                case invalid_object_boundary:
                    return "Invalid object boundary while slicing object";
            }

            return std::string{to_string()};
        }

    private:
        Value value = internal_error;
    };
}
