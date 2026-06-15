#pragma once

#include "ripper/pdf/core/document/object/object.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace ripper::pdf::core
{
/// Interface for serializing a single PDF object value into raw bytes.
///
/// This handles all concrete object types (null, bool, integer, real, string,
/// name, indirect reference, array, dictionary, stream) but not the indirect
/// object wrapper (`N G obj ... endobj`), which is the responsibility of
/// `indirect_object_serializer`.
class object_serializer
{
public:
    virtual ~object_serializer() = default;

    /// Serialize a PDF object value to a byte buffer.
    [[nodiscard]] virtual std::vector<std::byte> serialize(const object& obj) const = 0;

    /// Set the character used for line breaks in serialized output (default is `\n`).
    virtual void set_line_break_character(char c)
    {
        line_break_character_ = c;
    }

    /// Set the character used for mid-object breaks in serialized output (default is `\n`).
    virtual void set_object_break_character(char c)
    {
        object_break_character_ = c;
    }

protected:
    char line_break_character_ = '\n';
    char object_break_character_ = '\n';
};
} // namespace ripper::pdf::core
