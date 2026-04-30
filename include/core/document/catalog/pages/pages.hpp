#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/object/object.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class pages : public object
    {
    public:
        explicit pages(object object) noexcept;

        std::expected<std::uint64_t, error> count() const;
    };
}
