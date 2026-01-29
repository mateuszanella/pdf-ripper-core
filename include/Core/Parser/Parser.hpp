#pragma once

#include <expected>
#include <string>
#include <optional>

#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"
#include "Core/Parser/CrossReference/XrefTable.hpp"

namespace Ripper::Core
{
    class Parser
    {
    public:
        explicit Parser(Reader &reader);

        [[nodiscard]] std::expected<std::string, ParserError> ReadHeader();
        [[nodiscard]] std::expected<std::string, ParserError> ReadCrossReferenceTable();
        [[nodiscard]] std::expected<XrefTable, ParserError> ParseCrossReferenceTable();

        /**
         * @brief Returns the last computed breakpoints for this parser instance.
         */
        [[nodiscard]] const std::vector<Breakpoint> &GetBreakpoints() const;

    private:
        Reader &_reader;
        std::vector<Breakpoint> _breakpoints{};
    };
}
