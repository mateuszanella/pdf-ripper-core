#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ripper::core
{
    class error_code
    {
    public:
        // Generic, non-caller-actionable codes:
        enum Value : std::uint16_t
        {
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
            return lookup(value).name;
        }

        /// Return a human-readable default message for this error code.
        [[nodiscard]] std::string default_message() const
        {
            return std::string{lookup(value).message};
        }

    private:
        Value value = internal_error;

        struct entry { Value code; std::string_view name; std::string_view message; };

        static constexpr entry table[] = {
            { internal_error,              "internal_error",              "Unexpected internal failure"                                        },
            { invalid_argument,            "invalid_argument",            "Invalid input provided by caller"                                   },
            { not_found,                   "not_found",                   "Requested entity was not found"                                     },
            { io_error,                    "io_error",                    "Input/output failure"                                               },
            { parse_error,                 "parse_error",                 "Failure while parsing a PDF construct"                              },
            { missing_header,              "missing_header",              "PDF header was not found at expected location"                      },
            { corrupted_header,            "corrupted_header",            "PDF header is malformed"                                            },
            { missing_xref_table,          "missing_xref_table",          "Cross-reference table was not found in the document structure"      },
            { corrupted_xref_table,        "corrupted_xref_table",        "Cross-reference table is malformed"                                 },
            { missing_trailer,             "missing_trailer",             "Trailer was not found in the document structure"                    },
            { corrupted_trailer,           "corrupted_trailer",           "Trailer is malformed"                                               },
            { missing_catalog,             "missing_catalog",             "Catalog was not found in the document structure"                    },
            { corrupted_catalog,           "corrupted_catalog",           "Catalog is malformed"                                               },
            { corrupted_object,            "corrupted_object",            "Object is malformed"                                                },
            { object_not_found,            "object_not_found",            "Object was not found"                                               },
            { invalid_token,               "invalid_token",               "Invalid token encountered"                                          },
            { unterminated_literal_string, "unterminated_literal_string", "Literal string is not properly terminated"                          },
            { unterminated_hex_string,     "unterminated_hex_string",     "Hex string is not properly terminated"                              },
            { compression_invalid_input,   "compression_invalid_input",   "Input data is not valid for compression"                            },
            { compression_buffer_too_small,"compression_buffer_too_small","Compression buffer is too small"                                    },
            { compression_failed,          "compression_failed",          "Compression operation failed"                                       },
            { decompression_failed,        "decompression_failed",        "Decompression operation failed"                                     },
            { corrupted_compressed_data,   "corrupted_compressed_data",   "Compressed data is corrupted"                                       },
            { compression_memory_error,    "compression_memory_error",    "Memory error occurred during compression"                           },
            { reader_not_open,             "reader_not_open",             "Reader is not open"                                                 },
            { unexpected_eof,              "unexpected_eof",              "EOF reached before expected terminator"                             },
            { xref_entry_not_in_use,       "xref_entry_not_in_use",       "Cross-reference entry exists but is marked as free"                 },
            { xref_entry_not_found,        "xref_entry_not_found",        "Cross-reference entry for requested reference was not found"        },
            { generation_mismatch,         "generation_mismatch",         "Generation does not match requested reference"                      },
            { offset_out_of_bounds,        "offset_out_of_bounds",        "Requested offset is outside readable range"                         },
            { tokenization_error,          "tokenization_error",          "Lexer failed to produce a token"                                    },
            { invalid_object_boundary,     "invalid_object_boundary",     "Invalid object boundary while slicing object"                       },
        };

        [[nodiscard]] static constexpr const entry &lookup(Value v) noexcept
        {
            for (const auto &e : table)
                if (e.code == v) return e;
            return table[0];
        }
    };
}
