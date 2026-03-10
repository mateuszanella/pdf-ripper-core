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

    void catalog::set_outlines(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _outlinesObjectNumber = objectNumber;
        _outlinesGeneration = generation;
    }

    std::optional<std::uint32_t> catalog::outlines_object_number() const
    {
        return _outlinesObjectNumber;
    }

    std::optional<std::uint16_t> catalog::outlines_generation() const
    {
        return _outlinesGeneration;
    }

    void catalog::set_metadata(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _metadataObjectNumber = objectNumber;
        _metadataGeneration = generation;
    }

    std::optional<std::uint32_t> catalog::metadata_object_number() const
    {
        return _metadataObjectNumber;
    }

    std::optional<std::uint16_t> catalog::metadata_generation() const
    {
        return _metadataGeneration;
    }

    void catalog::set_lang(const std::string& lang)
    {
        _lang = lang;
    }

    const std::optional<std::string>& catalog::lang() const
    {
        return _lang;
    }
}
