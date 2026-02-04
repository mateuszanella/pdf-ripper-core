#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/Catalog/Catalog.hpp"
#include "Core/Errors/Parser/ParserError.hpp"

namespace Ripper::Core
{
    /**
     * @brief Interface for parsing catalog dictionaries.
     */
    class CatalogParser
    {
    public:
        virtual ~CatalogParser() = default;

        /**
         * @brief Parses a catalog dictionary from object content.
         * @param content The object content (between "obj" and "endobj")
         */
        [[nodiscard]] virtual std::expected<Catalog, ParserError> Parse(std::string_view content) = 0;
    };
}
