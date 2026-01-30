#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"

namespace Ripper::Core
{
    /**
     * @brief Interface for parsing trailer dictionaries.
     */
    class TrailerParser
    {
    public:
        virtual ~TrailerParser() = default;

        /**
         * @brief Parses a trailer dictionary from raw content.
         * @param content The raw trailer content (starting with "trailer" keyword)
         */
        [[nodiscard]] virtual std::expected<Trailer, ParserError> Parse(std::string_view content) = 0;
    };
}
