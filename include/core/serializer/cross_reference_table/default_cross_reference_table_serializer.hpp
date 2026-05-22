#pragma once

#include <cstddef>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"

namespace ripper::pdf::core
{
    /// Default implementation for serializing a PDF cross-reference table into raw bytes.
    ///
    /// Emits a traditional cross-reference table (`xref` keyword followed by one or more
    /// subsections of consecutive entries) per PDF spec §7.5.4.
    ///
    /// Entries are grouped into contiguous subsections automatically. Object 0 (the free-list
    /// head) is expected to already be present in the table.
    class default_cross_reference_table_serializer : public cross_reference_table_serializer
    {
    public:
        ~default_cross_reference_table_serializer() override = default;

        /// Serialize `xref` to a byte buffer.
        [[nodiscard]] std::vector<std::byte> serialize(const cross_reference_table &xref) const override;
    };
}
