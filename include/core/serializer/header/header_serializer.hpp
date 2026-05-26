#pragma once

#include <cstddef>
#include <vector>

#include "core/document/header.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// Interface for serializing a PDF `header` into raw bytes.
    class header_serializer
    {
    public:
        virtual ~header_serializer() = default;

        /// Serialize `header` to a byte buffer.
        [[nodiscard]] virtual std::vector<std::byte> serialize(const header &header) const = 0;

        /// Set the character used for line breaks in serialized output (default is `\n`).
        virtual void set_line_break_character(char c)
        {
            line_break_character_ = c;
        }

    protected:
        char line_break_character_ = '\n';
    };
}
