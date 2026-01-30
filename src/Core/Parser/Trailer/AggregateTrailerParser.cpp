#include "Core/Parser/Trailer/AggregateTrailerParser.hpp"

#include "Core/Parser/Trailer/DefaultTrailerParser.hpp"

namespace Ripper::Core
{
    AggregateTrailerParser::AggregateTrailerParser(Reader &reader, const std::vector<std::size_t> &xrefOffsets)
        : _reader{reader}, _xrefOffsets{xrefOffsets}
    {
    }

    std::expected<TrailerParseResult, ParserError> AggregateTrailerParser::Parse()
    {
        if (_xrefOffsets.empty())
        {
            return std::unexpected(ParserError::MissingTrailer);
        }

        Trailer mergedTrailer;
        std::vector<Breakpoint> allBreakpoints;

        // Parse trailers in order (newest to oldest)
        // Newer values will override older ones during merge
        for (std::size_t offset : _xrefOffsets)
        {
            _reader.Seek(offset);

            DefaultTrailerParser parser{_reader};
            auto parseResult = parser.Parse();
            if (!parseResult)
            {
                // If we can't find a trailer at this offset, continue to next
                // At least one trailer should be parseable
                continue;
            }

            mergedTrailer.Merge(parseResult->trailer);

            allBreakpoints.insert(allBreakpoints.end(),
                parseResult->breakpoints.begin(),
                parseResult->breakpoints.end());
        }

        return TrailerParseResult{
            .trailer = std::move(mergedTrailer),
            .breakpoints = std::move(allBreakpoints)
        };
    }
}
