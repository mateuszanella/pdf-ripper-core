#pragma once

#include <expected>
#include <string_view>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Trailer/TrailerParser.hpp"

namespace Ripper::Core
{
    /**
     * @brief Parses a single trailer dictionary.
     * Expects content starting with the "trailer" keyword.
     */
    class DefaultTrailerParser : public TrailerParser
    {
    public:
        DefaultTrailerParser() = default;

        [[nodiscard]] std::expected<Trailer, ParserError> Parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<Trailer, ParserError> ParseDictionary(std::string_view content);

        [[nodiscard]] static std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
            ParseIndirectReference(std::string_view line);
    };
}
