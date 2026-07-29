#pragma once

#include "ripper/pdf/core/document/revision.hpp"
#include "ripper/pdf/core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"
#include "ripper/pdf/core/serializer/trailer/trailer_serializer.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ripper::pdf::core
{

/// Serializes a single PDF revision: a cross-reference section paired with its trailer.
///
/// A revision is the unit of incremental update in a PDF file. It is composed of one
/// cross-reference section and one trailer dictionary. Depending on whether the section
/// is a compressed xref stream (PDF 1.5+, `rev.section().is_compressed()`) or a
/// traditional xref table, the serialized output differs:
///
///   - **Traditional**: `xref` block + `trailer` dictionary + `startxref\n<offset>\n%%EOF`
///   - **Compressed**:  xref stream indirect object (the trailer dictionary_object is merged
///     into the xref stream dictionary_object) + `startxref\n<offset>\n%%EOF`
///
/// The revision serializer delegates to the polymorphic `cross_reference_table_serializer`
/// (which dispatches to the default or compressed implementation based on the active
/// serializer) and to `trailer_serializer`. For compressed sections, the trailer
/// serializer is still invoked to emit the trailing `startxref` / `%%EOF` block, but
/// the trailer dictionary_object itself is emitted by the compressed xref serializer as part
/// of the stream dictionary, so the trailer serializer's dictionary_object output is not
/// appended.
///
/// `xref_offset` is the byte offset in the file at which the serialized xref block (or
/// xref stream indirect object) begins; it is written after `startxref` so reader
/// applications can locate the revision.
class revision_serializer
{
public:
    /// Construct a revision serializer bound to its sub-serializers.
    ///
    /// `xref_serializer` is the polymorphic cross-reference serializer (dispatches to
    /// the traditional or compressed implementation). `trailer_srl` serializes the
    /// `trailer\n<<dict>>\nstartxref\n<offset>\n%%EOF` block for traditional revisions
    /// and the `startxref\n<offset>\n%%EOF` tail for compressed revisions.
    explicit revision_serializer(cross_reference_table_serializer& xref_serializer,
                                 trailer_serializer& trailer_srl) noexcept;

    /// Serialize one revision to a byte buffer.
    ///
    /// When `rev.section().is_compressed()` is true, the trailer dictionary_object is merged
    /// into the xref stream dictionary_object by the compressed serializer, so only the
    /// trailing `startxref\n<offset>\n%%EOF` block is appended from the trailer
    /// serializer's `serialize_startxref()` output. Otherwise, the full trailer block
    /// (`trailer\n<<dict>>\nstartxref\n<offset>\n%%EOF`) is emitted.
    ///
    /// @param rev The revision to serialize.
    /// @param xref_offset The byte offset in the output file where the serialized xref
    ///                    block (or xref stream indirect object) begins.
    [[nodiscard]] std::vector<std::byte> serialize(const revision& rev,
                                                   std::uint64_t xref_offset) const;

private:
    cross_reference_table_serializer& xref_serializer_;
    trailer_serializer& trailer_serializer_;
};

} // namespace ripper::pdf::core
