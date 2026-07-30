#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"

#include <cstddef>
#include <vector>

namespace ripper::pdf::core
{
/// Interface for serializing a single PDF cross-reference section into raw bytes.
///
/// Operating on one section at a time allows callers to interleave object writes with
/// xref serialization, which is the required pattern for incremental PDF updates.
///
/// When the section is compressed (`is_compressed()` is true), the trailer dictionary
/// entries (Root, Info, ID, Encrypt, Prev) are merged into the xref stream dictionary,
/// mirroring the parser's 1:1 correspondence between in-memory state and file layout.
/// Implementations that emit a traditional `xref` table ignore the trailer argument.
class cross_reference_table_serializer
{
public:
    virtual ~cross_reference_table_serializer() = default;

    /// Serialize a single cross-reference section to a byte buffer.
    ///
    /// Emits the `xref` keyword followed by the subsection headers and 20-byte entries
    /// for `section` per PDF spec §7.5.4 (traditional implementation), or a compressed
    /// xref stream indirect object per PDF spec §7.5.8 (compressed implementation).
    ///
    /// `trailer` carries the trailer dictionary_object entries that the compressed serializer
    /// merges into the xref stream dictionary. The traditional serializer ignores it.
    [[nodiscard]] virtual std::vector<std::byte> serialize(const cross_reference_section& section,
                                                           const trailer& trailer) const = 0;
};
} // namespace ripper::pdf::core
