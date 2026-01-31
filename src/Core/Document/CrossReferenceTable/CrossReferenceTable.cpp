#include "Core/Document/CrossReferenceTable/CrossReferenceTable.hpp"

namespace Ripper::Core
{
    void CrossReferenceTable::AddEntry(std::uint32_t objectNumber, CrossReferenceEntry entry)
    {
        _entries[objectNumber] = entry;
    }

    const std::optional<CrossReferenceEntry> CrossReferenceTable::GetEntry(std::uint32_t objectNumber) const
    {
        auto it = _entries.find(objectNumber);
        if (it == _entries.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    const std::unordered_map<std::uint32_t, CrossReferenceEntry>& CrossReferenceTable::Entries() const
    {
        return _entries;
    }

    std::size_t CrossReferenceTable::Size() const
    {
        return _entries.size();
    }
}
