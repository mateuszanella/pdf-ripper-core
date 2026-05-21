#pragma once

#include <cstddef>
#include <vector>

#include "core/document/object/indirect_object.hpp"

namespace ripper::pdf::core
{
    /// Interface for serializing a PDF `indirect_object` into raw bytes.
    class indirect_object_serializer
    {
    public:
        virtual ~indirect_object_serializer() = default;

        /// Serialize `indirect_object` to a byte buffer.
        [[nodiscard]] virtual std::vector<std::byte> serialize(const indirect_object &obj) const = 0;

        /// Set the character used for mid-object breaks in serialized output (default is `\n`).
        void set_object_break_character(char line_break_char)
        {
            object_break_character_ = line_break_char;
        }

        /// Set the character used for line breaks in serialized output (default is `\n`).
        ///
        /// According to the PDF specification, line breaks in PDF content can be either
        /// LF (`\n`), CR (`\r`), or CRLF (`\r\n`).
        void set_line_break_character(char line_break_char)
        {
            line_break_character_ = line_break_char;
        }

    protected:
        char line_break_character_ = '\n';
        char object_break_character_ = '\n';
    };
}
