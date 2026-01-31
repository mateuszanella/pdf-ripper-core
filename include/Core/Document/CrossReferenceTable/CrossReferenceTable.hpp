#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "Core/Document/CrossReferenceTable/CrossReferenceEntry.hpp"

namespace Ripper::Core
{
    class CrossReferenceTable
    {
    public:
        CrossReferenceTable() = default;

        /**
         * @brief Adds or updates an entry in the cross-reference table.
         * Later entries override earlier ones for the same object number.
         */
        void AddEntry(std::uint32_t objectNumber, CrossReferenceEntry entry);

        /**
         * @brief Retrieves an entry for the given object number.
         */
        [[nodiscard]] const std::optional<CrossReferenceEntry> GetEntry(std::uint32_t objectNumber) const;

        /**
         * @brief Returns all entries in the table.
         */
        [[nodiscard]] const std::unordered_map<std::uint32_t, CrossReferenceEntry>& Entries() const;

        /**
         * @brief Returns the number of entries in the table.
         */
        [[nodiscard]] std::size_t Size() const;

    private:
        std::unordered_map<std::uint32_t, CrossReferenceEntry> _entries;
    };
}
