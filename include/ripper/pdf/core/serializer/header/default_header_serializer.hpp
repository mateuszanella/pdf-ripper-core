#pragma once

#include "ripper/pdf/core/document/header.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/serializer/header/header_serializer.hpp"

#include <cstddef>
#include <vector>

namespace ripper::pdf::core
{
/// Default implementation for serializing a PDF `header` into raw bytes.
class default_header_serializer : public header_serializer
{
public:
    virtual ~default_header_serializer() = default;

    /// Serialize `header` to a byte buffer.
    [[nodiscard]] std::vector<std::byte> serialize(const header& header) const override;
};
} // namespace ripper::pdf::core
