#pragma once

#include <cstddef>
#include <vector>

#include "core/document/header.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// Interface for serializing a PDF `header` into raw bytes.
    class header_serializer
    {
    public:
        virtual ~header_serializer() = default;

        /// Serialize `value` to a byte buffer.
        [[nodiscard]] virtual std::vector<std::byte> serialize(const header &value) const = 0;
    };
}
