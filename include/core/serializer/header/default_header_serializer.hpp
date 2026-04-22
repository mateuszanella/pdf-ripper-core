#pragma once

#include <cstddef>
#include <expected>
#include <vector>

#include "core/document/header.hpp"
#include "core/error.hpp"
#include "core/serializer/header/header_serializer.hpp"

namespace ripper::core
{
    /// Default implementation for serializing a PDF `header` into raw bytes.
    class default_header_serializer : public header_serializer
    {
    public:
        virtual ~default_header_serializer() = default;

        /// Serialize `value` to a byte buffer.
        [[nodiscard]] std::expected<std::vector<std::byte>, error> serialize(const header &value) const override;
    };
}
