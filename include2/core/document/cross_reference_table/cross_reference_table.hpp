#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_entry.hpp"

namespace ripper::core
{
    class cross_reference_table
    {
    public:
        cross_reference_table() = default;

        /**
         * @brief Adds or updates an entry in the cross-reference table.
         * Later entries override earlier ones for the same object number.
         */
        void add_entry(std::uint32_t objectNumber, cross_reference_entry entry);

        /**
         * @brief Retrieves an entry for the given object number.
         */
        [[nodiscard]] const std::optional<cross_reference_entry> get_entry(std::uint32_t objectNumber) const;

        /**
         * @brief Returns all entries in the table.
         */
        [[nodiscard]] const std::unordered_map<std::uint32_t, cross_reference_entry>& entries() const;

        /**
         * @brief Returns the number of entries in the table.
         */
        [[nodiscard]] std::size_t size() const;

    private:
        std::unordered_map<std::uint32_t, cross_reference_entry> _entries;
    };
}
