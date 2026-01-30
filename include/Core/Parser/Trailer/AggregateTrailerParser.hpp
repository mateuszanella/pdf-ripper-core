#pragma once

#include <expected>
#include <vector>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Parser/Trailer/TrailerParser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief Aggregates and parses all trailer dictionaries in a PDF document.
     *
     * This parser:
     * 1. Uses xref offsets to locate trailer positions
     * 2. Parses each trailer dictionary
     * 3. Merges results with proper override semantics (newer entries override older ones)
     */
    class AggregateTrailerParser : public TrailerParser
    {
    public:
        explicit AggregateTrailerParser(Reader &reader, const std::vector<std::size_t> &xrefOffsets);

        [[nodiscard]] std::expected<TrailerParseResult, ParserError> Parse() override;

    private:
        Reader &_reader;
        const std::vector<std::size_t> &_xrefOffsets;
    };
}
