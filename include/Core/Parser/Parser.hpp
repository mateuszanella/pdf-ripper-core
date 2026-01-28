#pragma once

#include <expected>
#include <string>
#include <optional>

#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class Parser
    {
    public:
        explicit Parser(Reader& reader);

        [[nodiscard]] std::expected<std::string, ParserError> ReadHeader();
        [[nodiscard]] std::expected<std::string, ParserError> ReadCrossReferenceTable();

        /**
         * @brief Breakpoint metadata captured during parsing of the PDF structure.
         *
         * All positions are byte offsets from the start of the file.
         */
        struct Breakpoints {
            std::optional<std::size_t> lastStartXrefKeywordPos;
            std::optional<std::size_t> lastStartXrefOffsetLinePos;
            std::optional<std::size_t> xrefOffset;
            std::optional<std::size_t> xrefStartPos;
            std::optional<std::size_t> xrefEndPos;
            std::optional<std::size_t> trailerKeywordPos;
            std::size_t eofPos = 0;
        };

        /**
         * @brief Returns the last computed breakpoints for this parser instance.
         */
        [[nodiscard]] const Breakpoints& GetBreakpoints() const;

    private:
        /**
         * @brief Scans the file to find the last startxref and parse its offset.
         *
         * Updates _breakpoints and returns the parsed offset.
         */
        [[nodiscard]] std::expected<std::size_t, ParserError> FindLastStartXrefOffset();

        /**
         * @brief Reads the xref table starting at the given offset until "trailer".
         *
         * Updates _breakpoints and returns the xref table bytes.
         */
        [[nodiscard]] std::expected<std::string, ParserError> ReadXrefTableAtOffset(std::size_t xrefOffset);

        Reader& _reader;
        Breakpoints _breakpoints{};
    };
}
