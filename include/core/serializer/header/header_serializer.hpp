#pragma once

#include <cstddef>
#include <expected>
#include <vector>

#include "core/document/header.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /// Interface for serializing a PDF `header` into raw bytes.
    class header_serializer
    {
    public:
        virtual ~header_serializer() = default;

        /// Serialize `value` to a byte buffer.
        [[nodiscard]] virtual std::expected<std::vector<const std::byte>, error> serialize(const header &value) const = 0;
    };
}
