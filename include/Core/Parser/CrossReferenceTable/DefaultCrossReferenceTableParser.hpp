#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/CrossReferenceTable/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/CrossReferenceTable/CrossReferenceTableParser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief Parses a single traditional (non-compressed) cross-reference table.
     * Expects content starting with the "xref" keyword.
     */
    class DefaultCrossReferenceTableParser : public CrossReferenceTableParser
    {
    public:
        DefaultCrossReferenceTableParser() = default;

        [[nodiscard]] std::expected<CrossReferenceTableParseResult, ParserError> Parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<void, ParserError> ParseSubsection(
            CrossReferenceTable &table,
            std::string_view &content);
    };
}
