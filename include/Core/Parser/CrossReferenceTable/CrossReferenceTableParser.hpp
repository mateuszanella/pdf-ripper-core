#pragma once

#include <expected>
#include <vector>

#include "Core/Document/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"

namespace Ripper::Core
{
    struct CrossReferenceTableParseResult
    {
        CrossReferenceTable table;
        std::vector<Breakpoint> breakpoints;
    };

    /**
     * @brief Interface for parsing cross-reference tables.
     * Implementations may parse a single table or aggregate multiple tables.
     */
    class CrossReferenceTableParser
    {
    public:
        virtual ~CrossReferenceTableParser() = default;

        /**
         * @brief Parses cross-reference table(s) and returns the result.
         */
        [[nodiscard]] virtual std::expected<CrossReferenceTableParseResult, ParserError> Parse() = 0;
    };
}
