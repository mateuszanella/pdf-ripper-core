#include "Core/Document/Catalog/Catalog.hpp"

namespace Ripper::Core
{
    void Catalog::SetPages(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _pagesObjectNumber = objectNumber;
        _pagesGeneration = generation;
    }

    std::optional<std::uint32_t> Catalog::PagesObjectNumber() const
    {
        return _pagesObjectNumber;
    }

    std::optional<std::uint16_t> Catalog::PagesGeneration() const
    {
        return _pagesGeneration;
    }

    void Catalog::SetOutlines(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _outlinesObjectNumber = objectNumber;
        _outlinesGeneration = generation;
    }

    std::optional<std::uint32_t> Catalog::OutlinesObjectNumber() const
    {
        return _outlinesObjectNumber;
    }

    std::optional<std::uint16_t> Catalog::OutlinesGeneration() const
    {
        return _outlinesGeneration;
    }

    void Catalog::SetMetadata(std::uint32_t objectNumber, std::uint16_t generation)
    {
        _metadataObjectNumber = objectNumber;
        _metadataGeneration = generation;
    }

    std::optional<std::uint32_t> Catalog::MetadataObjectNumber() const
    {
        return _metadataObjectNumber;
    }

    std::optional<std::uint16_t> Catalog::MetadataGeneration() const
    {
        return _metadataGeneration;
    }

    void Catalog::SetLang(const std::string& lang)
    {
        _lang = lang;
    }

    const std::optional<std::string>& Catalog::Lang() const
    {
        return _lang;
    }
}
