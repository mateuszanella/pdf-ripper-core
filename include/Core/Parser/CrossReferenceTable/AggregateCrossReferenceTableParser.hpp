#pragma once

#include <expected>
#include <memory>

#include "Core/Document/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Parser/CrossReferenceTable/CrossReferenceTableParser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief Aggregates and parses all cross-reference tables in a PDF document.
     *
     * This parser acts as a facade that:
     * 1. Locates all xref tables in the document (following the chain via /Prev entries)
     * 2. Delegates parsing of each individual table to appropriate parsers
     * 3. Merges results with proper override semantics (newer entries override older ones)
     */
    class AggregateCrossReferenceTableParser : public CrossReferenceTableParser
    {
    public:
        explicit AggregateCrossReferenceTableParser(Reader &reader);

        [[nodiscard]] std::expected<CrossReferenceTableParseResult, ParserError> Parse() override;

    private:
        Reader &_reader;

        [[nodiscard]] std::expected<std::vector<std::size_t>, ParserError> LocateAllXrefOffsets();
        [[nodiscard]] std::expected<std::size_t, ParserError> FindStartXrefOffset();
        [[nodiscard]] std::expected<std::size_t, ParserError> FindPrevXrefOffset(std::size_t currentXrefOffset);
        [[nodiscard]] std::unique_ptr<CrossReferenceTableParser> CreateParserForOffset(std::size_t offset);
    };
}
