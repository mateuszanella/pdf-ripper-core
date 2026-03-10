#pragma once

#include <expected>
#include <string_view>

#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    /**
     * @brief Interface for parsing catalog dictionaries.
     */
    class catalog_parser
    {
    public:
        virtual ~catalog_parser() = default;

        /**
         * @brief Parses a catalog dictionary from object content.
         * @param content The object content (between "obj" and "endobj")
         */
        [[nodiscard]] virtual std::expected<catalog, parser_error> parse(std::string_view content) = 0;
    };
}
