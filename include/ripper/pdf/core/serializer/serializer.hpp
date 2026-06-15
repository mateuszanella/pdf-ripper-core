#pragma once

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/serializer/serializer_manager.hpp"

namespace ripper::pdf::core
{
/// High-level PDF serializer facade for a single `document`.
///
/// This type orchestrates serialization by delegating to components managed by
/// `serializer_manager`, and throws on failures.
class serializer
{
public:
    /// Construct a serializer bound to `doc`.
    ///
    /// The serializer stores a reference and does not take ownership of the document.
    explicit serializer(const document& doc);

    /// Return the serializer manager used by this serializer.
    ///
    /// Can be used to replace serializer subcomponents.
    [[nodiscard]] class serializer_manager& manager();

    /// Set the character used for line breaks across all sub-serializers (default `\n`).
    void set_line_break_character(char c);

    /// Set the character used for object-level breaks across all sub-serializers (default `\n`).
    void set_object_break_character(char c);

    /// Serialize a PDF header to a byte buffer.
    [[nodiscard]] std::vector<std::byte> serialize_header(const header& header);

    /// Serialize a PDF indirect object to a byte buffer.
    [[nodiscard]] std::vector<std::byte> serialize_indirect_object(const indirect_object& obj);

    /// Serialize a single cross-reference section to a byte buffer.
    ///
    /// Emits the `xref` block for `section` only. Use this when interleaving object
    /// writes with xref writes during an incremental update:
    [[nodiscard]] std::vector<std::byte>
    serialize_cross_reference_section(const cross_reference_section& section);

    /// Serialize a PDF trailer block to a byte buffer.
    ///
    /// Emits `trailer\n<<dict>>\nstartxref\n<xref_offset>\n%%EOF\n`.
    /// Fields that are invalid in a full-save trailer (e.g. `/Prev`) are stripped.
    [[nodiscard]] std::vector<std::byte> serialize_trailer(const trailer& t,
                                                           std::uint64_t xref_offset);

private:
    const document& document_;

    std::unique_ptr<class serializer_manager> manager_;
};
} // namespace ripper::pdf::core
