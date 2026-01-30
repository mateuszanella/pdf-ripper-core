#pragma once

#include <expected>
#include <vector>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"

namespace Ripper::Core
{
    struct TrailerParseResult
    {
        Trailer trailer;
        std::vector<Breakpoint> breakpoints;
    };

    /**
     * @brief Interface for parsing trailer dictionaries.
     * Implementations may parse a single trailer or aggregate multiple trailers.
     */
    class TrailerParser
    {
    public:
        virtual ~TrailerParser() = default;

        /**
         * @brief Parses trailer(s) and returns the result.
         */
        [[nodiscard]] virtual std::expected<TrailerParseResult, ParserError> Parse() = 0;
    };
}
