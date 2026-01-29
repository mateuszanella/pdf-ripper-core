#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "Core/Document/Header.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    struct HeaderParseResult
    {
        Header header;
        std::vector<Breakpoint> breakpoints;
    };

    class HeaderParser
    {
    public:
        explicit HeaderParser(Reader &reader);

        [[nodiscard]] std::expected<HeaderParseResult, ParserError> Parse();

    private:
        Reader &_reader;
    };
}
