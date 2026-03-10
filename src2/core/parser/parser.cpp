#include "core/parser/parser.hpp"

#include "core/parser/header/header_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/catalog/default_catalog_parser.hpp"

namespace ripper::core
{
    parser::parser(reader &reader)
        : _reader{reader}
    {
    }

    std::expected<void, parser_error> parser::parse_header_if_needed()
    {
        if (_header.has_value())
        {
            return {};
        }

        header_parser headerParser{_reader};

        auto result = headerParser.parse();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        _header = std::move(result.value());

        return {};
    }

    std::expected<void, parser_error> parser::parse_structure_if_needed()
    {
        if (_structureParsed)
        {
            return {};
        }

        document_structure_parser structureParser{_reader};

        auto result = structureParser.parse();
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

    std::expected<void, parser_error> parser::parse_catalog_if_needed()
    {
        if (_catalog.has_value())
        {
            return {};
        }

        // Ensure structure is parsed first to get trailer
        auto structureResult = parse_structure_if_needed();
        if (!structureResult)
        {
            return structureResult;
        }

        // Get catalog object reference from trailer
        if (!_compiledTrailer->root_object_number())
        {
            return std::unexpected(parser_error::missing_catalog);
        }

        const std::uint32_t catalogObjNum = *_compiledTrailer->root_object_number();
        const std::uint16_t catalogGenNum = _compiledTrailer->root_generation().value_or(0);

        // Find catalog object in xref table
        const auto& entries = _compiledXrefTable->entries();
        auto it = entries.find(catalogObjNum);
        if (it == entries.end() || !it->second.in_use())
        {
            return std::unexpected(parser_error::missing_catalog);
        }

        const std::uint64_t catalogOffset = it->second.offset();

        // read catalog object from file
        _reader.seek(catalogOffset);

        constexpr std::size_t kBufferSize = 4096;
        std::array<std::byte, kBufferSize> buffer{};
        std::string objectContent;

        bool foundEndobj = false;
        while (!_reader.eof() && !foundEndobj)
        {
            const std::size_t bytesRead = _reader.read(buffer);
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
            return std::unexpected(parser_error::corrupted_catalog);
        }

        // parse catalog
        default_catalog_parser catalogParser;
        auto catalogResult = catalogParser.parse(objectContent);
        if (!catalogResult)
        {
            return std::unexpected(catalogResult.error());
        }

        _catalog = std::move(catalogResult.value());

        return {};
    }

    std::expected<void, parser_error> parser::ensure_parsed()
    {
        auto headerResult = parse_header_if_needed();
        if (!headerResult)
        {
            return headerResult;
        }

        return parse_structure_if_needed();
    }

    std::expected<header, parser_error> parser::header()
    {
        auto result = parse_header_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _header.value();
    }

    std::expected<cross_reference_table, parser_error> parser::cross_reference_table()
    {
        auto result = parse_structure_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _compiledXrefTable.value();
    }

    std::expected<std::vector<cross_reference_table>, parser_error> parser::cross_reference_table_history()
    {
        auto result = parse_structure_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _xrefTableHistory.value();
    }

    std::expected<trailer, parser_error> parser::trailer()
    {
        auto result = parse_structure_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _compiledTrailer.value();
    }

    std::expected<std::vector<trailer>, parser_error> parser::trailer_history()
    {
        auto result = parse_structure_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _trailerHistory.value();
    }

    std::expected<catalog, parser_error> parser::catalog()
    {
        auto result = parse_catalog_if_needed();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return _catalog.value();
    }
}
