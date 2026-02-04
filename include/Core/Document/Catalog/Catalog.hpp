#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace Ripper::Core
{
    /**
     * @brief Represents a PDF document catalog dictionary.
     *
     * The catalog is the root of the document's object hierarchy.
     * Referenced by the trailer's /Root entry.
     */
    class Catalog
    {
    public:
        Catalog() = default;

        void SetPages(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> PagesObjectNumber() const;
        [[nodiscard]] std::optional<std::uint16_t> PagesGeneration() const;

        void SetOutlines(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> OutlinesObjectNumber() const;
        [[nodiscard]] std::optional<std::uint16_t> OutlinesGeneration() const;

        void SetMetadata(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> MetadataObjectNumber() const;
        [[nodiscard]] std::optional<std::uint16_t> MetadataGeneration() const;

        void SetLang(const std::string& lang);
        [[nodiscard]] const std::optional<std::string>& Lang() const;

    private:
        std::optional<std::uint32_t> _pagesObjectNumber;
        std::optional<std::uint16_t> _pagesGeneration;
        std::optional<std::uint32_t> _outlinesObjectNumber;
        std::optional<std::uint16_t> _outlinesGeneration;
        std::optional<std::uint32_t> _metadataObjectNumber;
        std::optional<std::uint16_t> _metadataGeneration;
        std::optional<std::string> _lang;
    };
}
