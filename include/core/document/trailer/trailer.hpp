#pragma once

#include <cstdint>
#include <optional>

#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    /**
     * @brief Represents a PDF trailer dictionary.
     * Immutable after construction.
     */
    class trailer
    {
    public:
        struct builder
        {
            std::optional<std::uint64_t> size;
            std::optional<indirect_reference> root;
            std::optional<std::uint64_t> prev;

            [[nodiscard]] trailer build() const
            {
                return trailer{size.value_or(0), root, prev};
            }
        };

        [[nodiscard]] std::uint64_t size() const noexcept { return size_; }
        [[nodiscard]] const std::optional<indirect_reference> &root() const noexcept { return root_; }
        [[nodiscard]] const std::optional<std::uint64_t> &prev() const noexcept { return prev_; }

    private:
        trailer(std::uint64_t size, std::optional<indirect_reference> root, std::optional<std::uint64_t> prev) noexcept
            : size_{size}, root_{root}, prev_{prev}
        {
        }

        std::uint64_t size_;
        std::optional<indirect_reference> root_;
        std::optional<std::uint64_t> prev_;
    };
}
