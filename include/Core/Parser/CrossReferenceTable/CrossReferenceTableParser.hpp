#pragma once

#include <expected>

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
         * @brief Parses a single cross-reference table.
         * Reader should be positioned at the start of the xref section.
         */
        [[nodiscard]] virtual std::expected<CrossReferenceTableParseResult, ParserError> Parse() = 0;
    };
}
