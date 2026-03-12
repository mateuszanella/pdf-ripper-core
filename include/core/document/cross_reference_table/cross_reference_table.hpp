#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "core/document/indirect_reference.hpp"
#include "core/document/cross_reference_table/cross_reference_entry.hpp"

namespace ripper::core
{
    /**
     * @brief Compiled cross-reference table.
     * Immutable after construction.
     */
    class cross_reference_table
    {
    public:
        using entry_map = std::unordered_map<std::uint32_t, cross_reference_entry>;

        explicit cross_reference_table(entry_map entries) noexcept
            : entries_{std::move(entries)}
        {
        }

        [[nodiscard]] const entry_map &entries() const noexcept
        {
            return entries_;
        }

        /**
         * @brief Look up an entry by object number.
         */
        [[nodiscard]] std::optional<cross_reference_entry> find(std::uint32_t object_number) const
        {
            auto it = entries_.find(object_number);

            if (it == entries_.end())
            {
                return std::nullopt;
            }

            return it->second;
        }

        /**
         * @brief Look up an entry by indirect reference.
         */
        [[nodiscard]] std::optional<cross_reference_entry> find(const indirect_reference &ref) const
        {
            return find(ref.object_number());
        }

        [[nodiscard]] std::size_t size() const noexcept
        {
            return entries_.size();
        }

    private:
        entry_map entries_;
    };
}
