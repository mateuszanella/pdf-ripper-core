#pragma once

#include <cstddef>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_manager.hpp"

namespace ripper::pdf::core
{
    /// Interface for serializing a PDF cross-reference table into raw bytes.
    class cross_reference_table_serializer
    {
    public:
        virtual ~cross_reference_table_serializer() = default;

        /// Serialize `xref` to a byte buffer.
        [[nodiscard]] virtual std::vector<std::byte> serialize(const cross_reference_manager &xref) const = 0;
    };
}
