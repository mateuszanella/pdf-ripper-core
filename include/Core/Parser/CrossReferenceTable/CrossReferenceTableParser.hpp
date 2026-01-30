#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"

namespace Ripper::Core
{
    struct CrossReferenceTableParseResult
    {
        CrossReferenceTable table;
    };

    /**
     * @brief Interface for parsing cross-reference tables.
     * Implementations handle different xref formats (traditional, compressed streams).
     */
    class CrossReferenceTableParser
    {
    public:
        virtual ~CrossReferenceTableParser() = default;

        /**
         * @brief Parses a cross-reference table from raw content.
         * @param content The raw xref content (starting with "xref" keyword)
         */
        [[nodiscard]] virtual std::expected<CrossReferenceTableParseResult, ParserError> Parse(std::string_view content) = 0;
    };
}
