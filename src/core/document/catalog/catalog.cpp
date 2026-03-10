#include "core/document/catalog/catalog.hpp"

namespace ripper::core
{
    void catalog::set_pages(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _pagesObjectNumber = objectNumber;
        _pagesGeneration = generation;
    }

    std::optional<std::uint32_t> catalog::pages_object_number() const
    {
        return _pagesObjectNumber;
    }

    std::optional<std::uint16_t> catalog::pages_generation() const
    {
        return _pagesGeneration;
    }
}
