#include "core/parser/catalog/default_catalog_parser.hpp"

#include <optional>
#include <string_view>

#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<catalog_parse_result, parser_error> default_catalog_parser::parse_dictionary(
        std::string_view content,
        indirect_reference catalog_ref)
    {
        (void)content; // /Pages parsing/construction deferred for now.
        return catalog_parse_result{catalog_ref, std::nullopt};
    }

    std::expected<catalog_parse_result, parser_error> default_catalog_parser::parse(
        std::string_view content,
        indirect_reference catalog_ref) const
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

        return parse_dictionary(dictContent, catalog_ref);
    }
}
