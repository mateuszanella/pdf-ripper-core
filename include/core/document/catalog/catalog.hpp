#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace ripper::core
{
    /**
     * @brief Represents a pdf document catalog dictionary.
     *
     * The catalog is the root of the document's object hierarchy.
     * Referenced by the trailer's /Root entry.
     */
    class catalog
    {
    public:
        catalog() = default;

        void set_pages(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> pages_object_number() const;
        [[nodiscard]] std::optional<std::uint16_t> pages_generation() const;

        void set_outlines(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> outlines_object_number() const;
        [[nodiscard]] std::optional<std::uint16_t> outlines_generation() const;

        void set_metadata(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> metadata_object_number() const;
        [[nodiscard]] std::optional<std::uint16_t> metadata_generation() const;

        void set_lang(const std::string& lang);
        [[nodiscard]] const std::optional<std::string>& lang() const;

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
