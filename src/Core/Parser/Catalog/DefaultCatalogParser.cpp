#include "Core/Parser/Catalog/DefaultCatalogParser.hpp"

#include <string_view>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
        DefaultCatalogParser::ParseIndirectReference(std::string_view line)
    {
        const std::size_t firstSpace = line.find(' ');
        if (firstSpace == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        const auto objNum = Text::ParseSizeT(line.substr(0, firstSpace));
        if (!objNum)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        line = line.substr(firstSpace + 1);
        line = Text::TrimAscii(line);

        const std::size_t secondSpace = line.find(' ');
        if (secondSpace == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        const auto genNum = Text::ParseSizeT(line.substr(0, secondSpace));
        if (!genNum)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        return std::make_pair(
            static_cast<std::uint32_t>(*objNum),
            static_cast<std::uint16_t>(*genNum)
        );
    }

    std::expected<Catalog, ParserError> DefaultCatalogParser::ParseDictionary(
        std::string_view content)
    {
        Catalog catalog;

        // Parse /Pages
        if (const std::size_t pagesPos = content.find("/Pages"); pagesPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(pagesPos + 6);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                catalog.SetPages(ref->first, ref->second);
            }
        }

        // Parse /Outlines
        if (const std::size_t outlinesPos = content.find("/Outlines"); outlinesPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(outlinesPos + 9);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                catalog.SetOutlines(ref->first, ref->second);
            }
        }

        // Parse /Metadata
        if (const std::size_t metadataPos = content.find("/Metadata"); metadataPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(metadataPos + 9);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                catalog.SetMetadata(ref->first, ref->second);
            }
        }

        // Parse /Lang (string value)
        if (const std::size_t langPos = content.find("/Lang"); langPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(langPos + 5);
            rest = Text::TrimAscii(rest);

            // Look for literal string (...)
            if (const std::size_t strStart = rest.find('('); strStart != std::string_view::npos)
            {
                if (const std::size_t strEnd = rest.find(')', strStart + 1); strEnd != std::string_view::npos)
                {
                    catalog.SetLang(std::string{rest.substr(strStart + 1, strEnd - strStart - 1)});
                }
            }
        }

        return catalog;
    }

    std::expected<Catalog, ParserError> DefaultCatalogParser::Parse(std::string_view content)
    {
        // Find dictionary delimiters
        const std::size_t startPos = content.find("<<");
        const std::size_t endPos = content.find(">>");

        if (startPos == std::string_view::npos || endPos == std::string_view::npos || endPos <= startPos)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        std::string_view dictContent = content.substr(startPos + 2, endPos - startPos - 2);

        // Verify /Type /Catalog (optional but recommended)
        if (dictContent.find("/Type") != std::string_view::npos &&
            dictContent.find("/Catalog") == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedCatalog);
        }

        return ParseDictionary(dictContent);
    }
}
