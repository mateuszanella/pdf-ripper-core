#include "Core/Parser/Parser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string_view>
#include <stdexcept>
#include <mutex>
#include <unordered_map>

#include "Core/Document/Header.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"
#include "Core/Parser/Header/HeaderParser.hpp"
#include "Core/Parser/Breakpoint.hpp"
#include "Core/Parser/CrossReferenceTable/AggregateCrossReferenceTableParser.hpp"

namespace Ripper::Core
{
    Parser::Parser(Reader &reader)
        : _reader{reader}
    {
        _breakpoints.reserve(10);
    }

    std::expected<Header, ParserError> Parser::ParseHeader()
    {
        HeaderParser headerParser{_reader};
        auto result = headerParser.Parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _breakpoints.append_range(std::move(result->breakpoints));
        _header = std::move(result->header);

        return _header.value();
    }

    std::expected<CrossReferenceTable, ParserError> Parser::ParseCrossReferenceTable()
    {
        AggregateCrossReferenceTableParser parser{_reader};
        auto result = parser.Parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _breakpoints.append_range(std::move(result->breakpoints));
        _crossReferenceTable = std::move(result->table);

        return _crossReferenceTable.value();
    }

    std::expected<Header, ParserError> Parser::Header()
    {
        if (_header.has_value())
        {
            return _header.value();
        }

        return ParseHeader();
    }

    std::expected<CrossReferenceTable, ParserError> Parser::CrossReferenceTable()
    {
        if (_crossReferenceTable.has_value())
        {
            return _crossReferenceTable.value();
        }

        return ParseCrossReferenceTable();
    }

    const std::vector<Breakpoint> &Parser::Breakpoints() const
    {
        return _breakpoints;
    }
}
