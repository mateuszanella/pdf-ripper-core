#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/Catalog/Catalog.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Catalog/CatalogParser.hpp"

namespace Ripper::Core
{
    /**
     * @brief Parses a catalog dictionary from an indirect object.
     */
    class DefaultCatalogParser : public CatalogParser
    {
    public:
        DefaultCatalogParser() = default;

        [[nodiscard]] std::expected<Catalog, ParserError> Parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<Catalog, ParserError> ParseDictionary(std::string_view content);

        [[nodiscard]] static std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
            ParseIndirectReference(std::string_view line);
    };
}
