#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"

#include <charconv>
#include <string_view>

#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<void, parser_error> default_cross_reference_table_parser::parse_subsection(
        cross_reference_table::entry_map &entries,
        std::string_view &content)
    {
        // Find next newline to get header line
        const std::size_t newlinePos = content.find('\n');
        if (newlinePos == std::string_view::npos)
        {
            return std::unexpected(parser_error::unexpected_eof);
        }

        std::string_view headerLine = content.substr(0, newlinePos);
        headerLine = text::trim_ascii(text::strip_line_endings(headerLine));
        content = content.substr(newlinePos + 1);

        // parse start object number and count
        const std::size_t spacePos = headerLine.find(' ');
        if (spacePos == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_cross_reference_table);
        }

        const auto startObj = text::parse_size_t(headerLine.substr(0, spacePos));
        const auto count = text::parse_size_t(headerLine.substr(spacePos + 1));

        if (!startObj || !count)
        {
            return std::unexpected(parser_error::corrupted_cross_reference_table);
        }

        // parse entries
        for (std::size_t i = 0; i < *count; ++i)
        {
            const std::size_t entryNewline = content.find('\n');
            if (entryNewline == std::string_view::npos)
            {
                return std::unexpected(parser_error::unexpected_eof);
            }

            std::string_view entryLine = content.substr(0, entryNewline);
            entryLine = text::trim_ascii(text::strip_line_endings(entryLine));
            content = content.substr(entryNewline + 1);

            if (entryLine.size() < 18)
            {
                return std::unexpected(parser_error::corrupted_cross_reference_table);
            }

            // parse offset (10 digits)
            std::uint64_t offset = 0;
            auto [ptr1, ec1] = std::from_chars(
                entryLine.data(),
                entryLine.data() + 10,
                offset);

            if (ec1 != std::errc{})
            {
                return std::unexpected(parser_error::corrupted_cross_reference_table);
            }

            // parse generation (5 digits, starts at position 11)
            std::uint16_t generation = 0;
            auto [ptr2, ec2] = std::from_chars(
                entryLine.data() + 11,
                entryLine.data() + 16,
                generation);

            if (ec2 != std::errc{})
            {
                return std::unexpected(parser_error::corrupted_cross_reference_table);
            }

            // parse in-use flag (position 17)
            const char flag = entryLine[17];
            const bool inUse = (flag == 'n');

            const std::uint32_t objectNumber = static_cast<std::uint32_t>(*startObj + i);
            const indirect_reference ref{objectNumber, generation};

            entries.insert_or_assign(objectNumber, cross_reference_entry{ref, offset, inUse});
        }

        return {};
    }

    std::expected<cross_reference_table_parse_result, parser_error> default_cross_reference_table_parser::parse(
        std::string_view content)
    {
        cross_reference_table::entry_map entries;

        // Find first newline to get xref keyword line
        const std::size_t firstNewline = content.find('\n');
        if (firstNewline == std::string_view::npos)
        {
            return std::unexpected(parser_error::unexpected_eof);
        }

        std::string_view xrefLine = content.substr(0, firstNewline);
        xrefLine = text::trim_ascii(text::strip_line_endings(xrefLine));
        if (!text::starts_with_token(xrefLine, "xref"))
        {
            return std::unexpected(parser_error::missing_cross_reference_table);
        }

        content = content.substr(firstNewline + 1);

        // parse subsections until we hit "trailer"
        while (!content.empty())
        {
            content = text::trim_ascii(content);
            if (content.empty() || text::starts_with_token(content, "trailer"))
            {
                break;
            }

            auto result = parse_subsection(entries, content);
            if (!result)
            {
                return std::unexpected(result.error());
            }
        }

        cross_reference_table table{std::move(entries)};
        return cross_reference_table_parse_result{
            .table = std::move(table)
        };
    }
}
