#include "core/parser/catalog/default_catalog_parser.hpp"

#include <cstddef>
#include <string_view>

namespace ripper::core
{
    std::expected<indirect_reference, parser_error> default_catalog_parser::parse(
        std::string_view content,
        indirect_reference catalog_ref) const
    {
        // Find dictionary delimiters
        const std::size_t start_pos = content.find("<<");
        const std::size_t end_pos = content.rfind(">>");

        if (start_pos == std::string_view::npos || end_pos == std::string_view::npos || end_pos <= start_pos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        const std::string_view dict_content = content.substr(start_pos + 2, end_pos - start_pos - 2);

        // Verify /Type /Catalog (optional but recommended)
        if (dict_content.find("/Type") != std::string_view::npos &&
            dict_content.find("/Catalog") == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_catalog);
        }

        return catalog_ref;
    }
}
