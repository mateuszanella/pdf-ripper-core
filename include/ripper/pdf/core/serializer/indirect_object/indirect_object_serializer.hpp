#pragma once

#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/serializer/object/object_serializer.hpp"

#include <cstddef>
#include <vector>

namespace ripper::pdf::core
{
/// Interface for serializing a PDF `indirect_object` into raw bytes.
class indirect_object_serializer
{
public:
    virtual ~indirect_object_serializer() = default;

    /// Serialize `indirect_object` to a byte buffer.
    [[nodiscard]] virtual std::vector<std::byte> serialize(const indirect_object& obj) const = 0;

    /// Rebind the object value serializer used internally.
    ///
    /// The default implementation is a no-op; concrete subclasses should override this.
    virtual void set_object_serializer(class object_serializer& serializer) {}

    /// Set the character used for mid-object breaks in serialized output (default is `\n`).
    virtual void set_object_break_character(char c)
    {
        object_break_character_ = c;
    }

    /// Set the character used for line breaks in serialized output (default is `\n`).
    ///
    /// According to the PDF specification, line breaks in PDF content can be either
    /// LF (`\n`), CR (`\r`), or CRLF (`\r\n`).
    virtual void set_line_break_character(char c)
    {
        line_break_character_ = c;
    }

protected:
    char line_break_character_ = '\n';
    char object_break_character_ = '\n';
};
} // namespace ripper::pdf::core
