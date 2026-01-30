#include "Core/Parser/Parser.hpp"

#include "Core/Parser/Header/HeaderParser.hpp"
#include "Core/Parser/DocumentStructure/DocumentStructureParser.hpp"

namespace Ripper::Core
{
    Parser::Parser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<void, ParserError> Parser::ParseHeaderIfNeeded()
    {
        if (_header.has_value())
        {
            return {};
        }

        HeaderParser headerParser{_reader};

        auto result = headerParser.Parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _header = std::move(result.value());

        return {};
    }

    std::expected<void, ParserError> Parser::ParseStructureIfNeeded()
    {
        if (_structureParsed)
        {
            return {};
        }

        DocumentStructureParser structureParser{_reader};

        auto result = structureParser.Parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _compiledXrefTable = std::move(result->compiledXrefTable);
        _xrefTableHistory = std::move(result->xrefTableHistory);
        _compiledTrailer = std::move(result->compiledTrailer);
        _trailerHistory = std::move(result->trailerHistory);
        _structureParsed = true;

        return {};
    }

    std::expected<void, ParserError> Parser::EnsureParsed()
    {
        auto headerResult = ParseHeaderIfNeeded();
        if (!headerResult)
        {
            return headerResult;
        }

        return ParseStructureIfNeeded();
    }

    std::expected<Header, ParserError> Parser::Header()
    {
        auto result = ParseHeaderIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _header.value();
    }

    std::expected<CrossReferenceTable, ParserError> Parser::CrossReferenceTable()
    {
        auto result = ParseStructureIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _compiledXrefTable.value();
    }

    std::expected<std::vector<CrossReferenceTable>, ParserError> Parser::CrossReferenceTableHistory()
    {
        auto result = ParseStructureIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _xrefTableHistory.value();
    }

    std::expected<Trailer, ParserError> Parser::Trailer()
    {
        auto result = ParseStructureIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _compiledTrailer.value();
    }

    std::expected<std::vector<Trailer>, ParserError> Parser::TrailerHistory()
    {
        auto result = ParseStructureIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _trailerHistory.value();
    }
}
