#pragma once

#include <expected>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Parser/Trailer/TrailerParser.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief Parses a single trailer dictionary.
     * Expects the reader to be positioned at or before the "trailer" keyword.
     */
    class DefaultTrailerParser : public TrailerParser
    {
    public:
        explicit DefaultTrailerParser(Reader &reader);

        [[nodiscard]] std::expected<TrailerParseResult, ParserError> Parse() override;

    private:
        Reader &_reader;

        [[nodiscard]] std::expected<void, ParserError> ParseDictionary(
            Trailer &trailer,
            std::vector<Breakpoint> &breakpoints);

        [[nodiscard]] std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
            ParseIndirectReference(std::string_view line);
    };
}
