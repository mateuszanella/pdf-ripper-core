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

namespace Ripper::Core
{
    Parser::Parser(Reader &reader)
        : _reader{reader}
    {
        _breakpoints.reserve(10);
    }

    std::expected<Header, ParserError> Parser::ParseHeader()
    {
        if (_header)
        {
            return *_header;
        }

        HeaderParser headerParser{_reader};
        auto result = headerParser.Parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _breakpoints.append_range(std::move(result->breakpoints));
        _header = std::move(result->header);

        return *_header;
    }

    const std::vector<Breakpoint> &Parser::Breakpoints() const
    {
        return _breakpoints;
    }

    const std::optional<Header> &Parser::Header() const
    {
        return _header;
    }
}
