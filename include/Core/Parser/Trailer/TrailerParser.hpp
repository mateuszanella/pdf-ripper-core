#pragma once

#include <expected>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"

namespace Ripper::Core
{
    struct TrailerParseResult
    {
        Trailer trailer;
    };

    /**
     * @brief Interface for parsing trailer dictionaries.
     */
    class TrailerParser
    {
    public:
        virtual ~TrailerParser() = default;

        /**
         * @brief Parses a single trailer dictionary.
         * Reader should be positioned at or before the "trailer" keyword.
         */
        [[nodiscard]] virtual std::expected<TrailerParseResult, ParserError> Parse() = 0;
    };
}
