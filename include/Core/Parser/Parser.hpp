#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "Core/Document/Header.hpp"
#include "Core/Document/CrossReferenceTable.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class Parser
    {
    public:
        explicit Parser(Reader &reader);

        /**
         * @brief Force parsing of the PDF header from the underlying reader.
         *
         * @return The parsed Header on success, or a ParserError on failure.
         */
        [[nodiscard]] std::expected<Header, ParserError> ParseHeader();

        /**
         * @brief Force parsing of the cross-reference table from the underlying reader.
         *
         * @return The parsed CrossReferenceTable on success, or a ParserError on failure.
         */
        [[nodiscard]] std::expected<CrossReferenceTable, ParserError> ParseCrossReferenceTable();

        /**
         * @brief Returns the last computed header for this parser instance.
         * If the header has not been parsed yet, it triggers parsing.
         */
        [[nodiscard]] std::expected<Header, ParserError> Header();

        /**
         * @brief Returns the last computed cross-reference table for this parser instance.
         * If the table has not been parsed yet, it triggers parsing.
         */
        [[nodiscard]] std::expected<CrossReferenceTable, ParserError> CrossReferenceTable();

        /**
         * @brief Returns the last computed breakpoints for this parser instance.
         */
        [[nodiscard]] const std::vector<Breakpoint> &Breakpoints() const;

    private:
        Reader &_reader;
        std::vector<Breakpoint> _breakpoints{};

        std::optional<class Header> _header;
        std::optional<class CrossReferenceTable> _crossReferenceTable;
    };
}
