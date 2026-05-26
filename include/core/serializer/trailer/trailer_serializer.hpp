#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/document/trailer/trailer.hpp"
#include "core/serializer/object/object_serializer.hpp"

namespace ripper::pdf::core
{
    /// Interface for serializing a PDF trailer block into raw bytes.
    ///
    /// Produces the `trailer\n<<...>>\nstartxref\n<offset>\n%%EOF\n` block.
    class trailer_serializer
    {
    public:
        virtual ~trailer_serializer() = default;

        /// Serialize `t` to a byte buffer.
        ///
        /// `xref_offset` is the byte offset of the preceding `xref` keyword in the file.
        [[nodiscard]] virtual std::vector<std::byte> serialize(const trailer &t, std::uint64_t xref_offset) const = 0;

        /// Rebind the object value serializer used internally.
        ///
        /// The default implementation is a no-op; concrete subclasses should override this.
        virtual void set_object_serializer(class object_serializer &serializer) {}

        /// Set the character used for line breaks in serialized output (default is `\n`).
        virtual void set_line_break_character(char c)
        {
            line_break_character_ = c;
        }

    protected:
        char line_break_character_ = '\n';
    };
}
