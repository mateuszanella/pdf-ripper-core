#include "Core/Document/CrossReferenceEntry.hpp"

namespace Ripper::Core
{
    CrossReferenceEntry::CrossReferenceEntry(std::uint64_t offset, std::uint16_t generation, bool inUse)
        : _offset{offset}
        , _generation{generation}
        , _inUse{inUse}
    {
    }
}
