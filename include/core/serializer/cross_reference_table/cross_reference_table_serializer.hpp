#pragma once

#include "core/document/cross_reference_table/cross_reference_section.hpp"

#include <cstddef>
#include <vector>

namespace ripper::pdf::core
{
/// Interface for serializing a single PDF cross-reference section into raw bytes.
///
/// Operating on one section at a time allows callers to interleave object writes with
/// xref serialization, which is the required pattern for incremental PDF updates.
class cross_reference_table_serializer
{
public:
    virtual ~cross_reference_table_serializer() = default;

    /// Serialize a single cross-reference section to a byte buffer.
    ///
    /// Emits the `xref` keyword followed by the subsection headers and 20-byte entries
    /// for `section` per PDF spec §7.5.4.
    [[nodiscard]] virtual std::vector<std::byte>
    serialize(const cross_reference_section& section) const = 0;
};
} // namespace ripper::pdf::core
