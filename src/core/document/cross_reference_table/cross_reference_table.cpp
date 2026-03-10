#include "core/document/cross_reference_table/cross_reference_table.hpp"

namespace ripper::core
{
    void cross_reference_table::add_entry(std::uint32_t objectNumber, cross_reference_entry entry)
    {
        _entries[objectNumber] = entry;
    }

    const std::optional<cross_reference_entry> cross_reference_table::get_entry(std::uint32_t objectNumber) const
    {
        auto it = _entries.find(objectNumber);
        if (it == _entries.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    const std::unordered_map<std::uint32_t, cross_reference_entry>& cross_reference_table::entries() const
    {
        return _entries;
    }

    std::size_t cross_reference_table::size() const
    {
        return _entries.size();
    }
}
