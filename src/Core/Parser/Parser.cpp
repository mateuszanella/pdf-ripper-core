#include "Core/Parser/Parser.hpp"

#include "Core/Parser/Header/HeaderParser.hpp"
#include "Core/Parser/DocumentStructure/DocumentStructureParser.hpp"
#include "Core/Parser/Catalog/DefaultCatalogParser.hpp"

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

    std::expected<void, ParserError> Parser::ParseCatalogIfNeeded()
    {
        if (_catalog.has_value())
        {
            return {};
        }

        // Ensure structure is parsed first to get trailer
        auto structureResult = ParseStructureIfNeeded();
        if (!structureResult)
        {
            return structureResult;
        }

        // Get catalog object reference from trailer
        if (!_compiledTrailer->RootObjectNumber())
        {
            return std::unexpected(ParserError::MissingCatalog);
        }

        const std::uint32_t catalogObjNum = *_compiledTrailer->RootObjectNumber();
        const std::uint16_t catalogGenNum = _compiledTrailer->RootGeneration().value_or(0);

        // Find catalog object in xref table
        const auto& entries = _compiledXrefTable->Entries();
        auto it = entries.find(catalogObjNum);
        if (it == entries.end() || !it->second.InUse())
        {
            return std::unexpected(ParserError::MissingCatalog);
        }

        const std::uint64_t catalogOffset = it->second.Offset();

        // Read catalog object from file
        _reader.Seek(catalogOffset);

        constexpr std::size_t kBufferSize = 4096;
        std::array<std::byte, kBufferSize> buffer{};
        std::string objectContent;

        bool foundEndobj = false;
        while (!_reader.Eof() && !foundEndobj)
        {
            const std::size_t bytesRead = _reader.Read(buffer);
            if (bytesRead == 0)
            {
                break;
            }

            std::string_view chunk{
                reinterpret_cast<const char*>(buffer.data()),
                bytesRead};
            objectContent += chunk;

            if (objectContent.find("endobj") != std::string::npos)
            {
                foundEndobj = true;
            }
        }

        if (!foundEndobj)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        // Parse catalog
        DefaultCatalogParser catalogParser;
        auto catalogResult = catalogParser.Parse(objectContent);
        if (!catalogResult)
        {
            return std::unexpected(catalogResult.error());
        }

        _catalog = std::move(catalogResult.value());

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

    std::expected<Catalog, ParserError> Parser::Catalog()
    {
        auto result = ParseCatalogIfNeeded();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _catalog.value();
    }
}
