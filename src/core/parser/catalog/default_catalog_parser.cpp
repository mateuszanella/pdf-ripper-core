#include "core/parser/catalog/default_catalog_parser.hpp"

#include <string_view>

#include "core/util/text.hpp"

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

        // Verify /Type /Catalog (optional but recommended)
        if (dictContent.find("/Type") != std::string_view::npos &&
            dictContent.find("/Catalog") == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        return parse_dictionary(dictContent);
    }
}
