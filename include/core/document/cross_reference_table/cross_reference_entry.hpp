#pragma once

#include <cstdint>

#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    /**
     * @brief A single entry in a cross-reference table.
     * Maps an indirect reference to a byte offset and in-use flag.
     */
    class cross_reference_entry
    {
    public:
        constexpr cross_reference_entry(indirect_reference ref, std::uint64_t offset, bool in_use) noexcept
            : reference_{ref}, offset_{offset}, in_use_{in_use}
        {
        }

        [[nodiscard]] constexpr const indirect_reference &reference() const noexcept { return reference_; }
        [[nodiscard]] constexpr std::uint64_t offset() const noexcept { return offset_; }
        [[nodiscard]] constexpr bool in_use() const noexcept { return in_use_; }

    private:
        indirect_reference reference_;
        std::uint64_t offset_;
        bool in_use_;
    };
}
