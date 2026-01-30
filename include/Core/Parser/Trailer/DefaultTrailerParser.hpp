#pragma once

#include <expected>

#include "Core/Document/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
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

        [[nodiscard]] std::expected<Trailer, ParserError> Parse() override;

    private:
        Reader &_reader;

        [[nodiscard]] static std::expected<Trailer, ParserError> ParseDictionary(std::string_view content);

        [[nodiscard]] static std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
            ParseIndirectReference(std::string_view line);
    };
}
