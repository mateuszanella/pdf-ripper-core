#pragma once

#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/serializer/object/object_serializer.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
/// Interface for serializing a PDF trailer block into raw bytes.
///
/// Produces the `trailer\n<<...>>\nstartxref\n<offset>\n%%EOF\n` block for
/// traditional revisions, or just `startxref\n<offset>\n%%EOF\n` for
/// compressed xref stream revisions (where the trailer dictionary_object is merged
/// into the xref stream dictionary_object).
class trailer_serializer
{
public:
    virtual ~trailer_serializer() = default;

    /// Serialize `t` to a byte buffer.
    ///
    /// `xref_offset` is the byte offset of the preceding `xref` keyword in the file.
    [[nodiscard]] virtual std::vector<std::byte> serialize(const trailer& t,
                                                           std::uint64_t xref_offset) const = 0;

    /// Serialize only the trailing `startxref\n<offset>\n%%EOF\n` block.
    ///
    /// Used by `revision_serializer` for compressed xref stream revisions, where
    /// the trailer dictionary_object is merged into the xref stream dictionary_object and no
    /// separate `trailer` keyword is emitted. The default implementation produces
    /// `startxref\n<xref_offset>\n%%EOF\n` using `line_break_character_`.
    [[nodiscard]] virtual std::vector<std::byte>
    serialize_startxref(std::uint64_t xref_offset) const
    {
        std::vector<std::byte> out;
        const auto c = static_cast<std::byte>(line_break_character_);
        const std::string offset_str = std::to_string(xref_offset);

        for (const char ch : std::string_view{"startxref"})
            out.push_back(static_cast<std::byte>(ch));
        out.push_back(c);
        for (const char ch : offset_str)
            out.push_back(static_cast<std::byte>(ch));
        out.push_back(c);
        for (const char ch : std::string_view{"%%EOF"})
            out.push_back(static_cast<std::byte>(ch));
        out.push_back(c);

        return out;
    }

    /// Rebind the object value serializer used internally.
    ///
    /// The default implementation is a no-op; concrete subclasses should override this.
    virtual void set_object_serializer(class object_serializer& serializer) {}

    /// Set the character used for line breaks in serialized output (default is `\n`).
    virtual void set_line_break_character(char c)
    {
        line_break_character_ = c;
    }

protected:
    char line_break_character_ = '\n';
};
} // namespace ripper::pdf::core
