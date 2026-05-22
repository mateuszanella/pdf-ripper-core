#pragma once

#include <cstddef>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a PDF cross-reference table into raw bytes.
    ///
    /// Emits a traditional cross-reference table (`xref` keyword followed by one or more
    /// subsections) per PDF spec §7.5.4. Subsection grouping is provided directly by the
    /// `cross_reference_manager` via its sections and subsections.
    ///
    /// Object 0 (the free-list head) is expected to already be present in the table.
    class default_cross_reference_table_serializer : public cross_reference_table_serializer
    {
    public:
        ~default_cross_reference_table_serializer() override = default;

        /// Serialize `xref` to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const cross_reference_manager &xref) const override;
    };
}
