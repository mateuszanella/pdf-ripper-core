#include "core/document/cross_reference_table/cross_reference_entry.hpp"

namespace ripper::core
{
    cross_reference_entry::cross_reference_entry(std::uint64_t offset, std::uint16_t generation, bool inUse)
        : _offset{offset}
        , _generation{generation}
        , _inUse{inUse}
    {
    }
}
