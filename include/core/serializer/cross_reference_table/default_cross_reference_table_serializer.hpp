#pragma once

#include <cstddef>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_section.hpp"
#include "core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a single PDF cross-reference section into raw bytes.
    ///
    /// Emits a traditional cross-reference section (`xref` keyword followed by one or more
    /// subsection headers and 20-byte entries) per PDF spec §7.5.4. Operating per section
    /// rather than per manager allows callers to interleave object writes with xref writes,
    /// which is required for incremental PDF updates.
    ///
    /// Object 0 (the free-list head) is expected to already be present in the section.
    class default_cross_reference_table_serializer : public cross_reference_table_serializer
    {
    public:
        ~default_cross_reference_table_serializer() override = default;

        /// Serialize a single cross-reference section to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const cross_reference_section &section) const override;
    };
}
