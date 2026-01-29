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
         * @brief Parses the PDF header from the underlying reader.
         *
         * @return The parsed Header on success, or a ParserError on failure.
         */
        [[nodiscard]] std::expected<Header, ParserError> ParseHeader();

        /**
         * @brief Parses the cross-reference table from the underlying reader.
         *
         * @return The parsed CrossReferenceTable on success, or a ParserError on failure.
         */
        [[nodiscard]] std::expected<CrossReferenceTable, ParserError> ParseCrossReferenceTable();

        /**
         * @brief Returns the last parsed header, if any.
         */
        [[nodiscard]] const std::optional<Header> &Header() const;

        /**
         * @brief Returns the last parsed cross-reference table, if any.
         */
        [[nodiscard]] const std::optional<CrossReferenceTable> &CrossReferenceTable() const;

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
