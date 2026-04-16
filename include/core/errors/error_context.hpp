#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "core/errors/error_component.hpp"

namespace ripper::core
{
    /// Typed diagnostic metadata attached to an `error`.
    struct error_context
    {
        error_component component{error_component::unknown};
        std::optional<std::size_t> offset{};
        std::optional<std::uint32_t> object_number{};
        std::optional<std::uint16_t> generation{};
        std::optional<std::string> field{};
        std::optional<std::string> expected{};
        std::optional<std::string> actual{};

        [[nodiscard]] bool empty() const noexcept
        {
                 return component == error_component{error_component::unknown} &&
                   !offset.has_value() &&
                   !object_number.has_value() &&
                   !generation.has_value() &&
                   !field.has_value() &&
                   !expected.has_value() &&
                   !actual.has_value();
        }
    };
}
