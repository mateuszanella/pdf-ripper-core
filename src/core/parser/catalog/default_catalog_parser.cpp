#include "core/parser/catalog/default_catalog_parser.hpp"

#include <string_view>

#include "core/Util/Text.hpp"

namespace ripper::core
{
    std::expected<std::pair<std::uint32_t, std::uint16_t>, parser_error>
        default_catalog_parser::parse_indirect_reference(std::string_view line)
    {
        const std::size_t firstSpace = line.find(' ');
        if (firstSpace == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        const auto objNum = text::parse_size_t(line.substr(0, firstSpace));
        if (!objNum)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        line = line.substr(firstSpace + 1);
        line = text::trim_ascii(line);

        const std::size_t secondSpace = line.find(' ');
        if (secondSpace == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        const auto genNum = text::parse_size_t(line.substr(0, secondSpace));
        if (!genNum)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        return std::make_pair(
            static_cast<std::uint32_t>(*objNum),
            static_cast<std::uint16_t>(*genNum)
        );
    }

    std::expected<catalog, parser_error> default_catalog_parser::parse_dictionary(
        std::string_view content)
    {
        catalog catalog;

        // parse /Pages
        if (const std::size_t pagesPos = content.find("/Pages"); pagesPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(pagesPos + 6);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                catalog.set_pages(ref->first, ref->second);
            }
        }

        // parse /Outlines
        if (const std::size_t outlinesPos = content.find("/Outlines"); outlinesPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(outlinesPos + 9);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                catalog.set_outlines(ref->first, ref->second);
            }
        }

        // parse /Metadata
        if (const std::size_t metadataPos = content.find("/Metadata"); metadataPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(metadataPos + 9);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                catalog.set_metadata(ref->first, ref->second);
            }
        }

        // parse /lang (string value)
        if (const std::size_t langPos = content.find("/Lang"); langPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(langPos + 5);
            rest = text::trim_ascii(rest);

            // Look for literal string (...)
            if (const std::size_t strStart = rest.find('('); strStart != std::string_view::npos)
            {
                if (const std::size_t strEnd = rest.find(')', strStart + 1); strEnd != std::string_view::npos)
                {
                    catalog.set_lang(std::string{rest.substr(strStart + 1, strEnd - strStart - 1)});
                }
            }
        }

        return catalog;
    }

    std::expected<catalog, parser_error> default_catalog_parser::parse(std::string_view content)
    {
        // Find dictionary delimiters
        const std::size_t startPos = content.find("<<");
        const std::size_t endPos = content.find(">>");

        if (startPos == std::string_view::npos || endPos == std::string_view::npos || endPos <= startPos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        std::string_view dictContent = content.substr(startPos + 2, endPos - startPos - 2);

        // Verify /Type /catalog (optional but recommended)
        if (dictContent.find("/Type") != std::string_view::npos &&
            dictContent.find("/catalog") == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        return parse_dictionary(dictContent);
    }
}
