#pragma once

#include <expected>

#include "Core/Document/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Parser/CrossReferenceTable/CrossReferenceTableParser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief Parses a single traditional (non-compressed) cross-reference table.
     * Expects the reader to be positioned at the start of an "xref" keyword.
     */
    class DefaultCrossReferenceTableParser : public CrossReferenceTableParser
    {
    public:
        explicit DefaultCrossReferenceTableParser(Reader &reader);

        [[nodiscard]] std::expected<CrossReferenceTableParseResult, ParserError> Parse() override;

    private:
        Reader &_reader;

        [[nodiscard]] std::expected<void, ParserError> ParseSubsection(
            CrossReferenceTable &table,
            std::vector<Breakpoint> &breakpoints);
    };
}
