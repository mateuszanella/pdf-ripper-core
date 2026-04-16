#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"

#include <charconv>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "core/error_builder.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<void, error> default_cross_reference_table_parser::parse_subsection(
        cross_reference_table::entry_map &entries,
        std::string_view &content)
    {
        const std::size_t newlinePos = content.find('\n');
        if (newlinePos == std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unexpected_eof)
                                       .with_component(error_component::cross_reference)
                                       .with_field("subsection_header")
                                       .with_message("Unexpected EOF while parsing xref subsection header")
                                       .build());
        }

        std::string_view headerLine = content.substr(0, newlinePos);
        headerLine = text::trim_ascii(text::strip_line_endings(headerLine));
        content = content.substr(newlinePos + 1);

        const std::size_t spacePos = headerLine.find(' ');
        if (spacePos == std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_xref_table)
                                       .with_component(error_component::cross_reference)
                                       .with_field("subsection_header")
                                       .with_expected("<start> <count>")
                                       .with_actual(std::string{headerLine})
                                       .with_message("Invalid xref subsection header")
                                       .build());
        }

        const auto startObj = text::parse_size_t(headerLine.substr(0, spacePos));
        const auto count = text::parse_size_t(headerLine.substr(spacePos + 1));

        if (!startObj || !count)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_xref_table)
                                       .with_component(error_component::cross_reference)
                                       .with_field("subsection_range")
                                       .with_actual(std::string{headerLine})
                                       .with_message("Invalid xref subsection range")
                                       .build());
        }

        for (std::size_t i = 0; i < *count; ++i)
        {
            const std::size_t entryNewline = content.find('\n');
            if (entryNewline == std::string_view::npos)
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::unexpected_eof)
                                           .with_component(error_component::cross_reference)
                                           .with_field("entry")
                                           .with_message("Unexpected EOF while parsing xref entries")
                                           .build());
            }

            std::string_view entryLine = content.substr(0, entryNewline);
            entryLine = text::trim_ascii(text::strip_line_endings(entryLine));
            content = content.substr(entryNewline + 1);

            if (entryLine.size() < 18)
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::corrupted_xref_table)
                                           .with_component(error_component::cross_reference)
                                           .with_field("entry")
                                           .with_expected("<offset:10> <generation:5> <flag>")
                                           .with_actual(std::string{entryLine})
                                           .with_message("Malformed xref entry")
                                           .build());
            }

            std::uint64_t offset = 0;
            auto [ptr1, ec1] = std::from_chars(
                entryLine.data(),
                entryLine.data() + 10,
                offset);

            if (ec1 != std::errc{})
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::corrupted_xref_table)
                                           .with_component(error_component::cross_reference)
                                           .with_field("entry_offset")
                                           .with_actual(std::string{entryLine.substr(0, 10)})
                                           .with_message("Invalid xref entry offset")
                                           .build());
            }

            std::uint16_t generation = 0;
            auto [ptr2, ec2] = std::from_chars(
                entryLine.data() + 11,
                entryLine.data() + 16,
                generation);

            if (ec2 != std::errc{})
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::corrupted_xref_table)
                                           .with_component(error_component::cross_reference)
                                           .with_field("entry_generation")
                                           .with_actual(std::string{entryLine.substr(11, 5)})
                                           .with_message("Invalid xref entry generation")
                                           .build());
            }

            const char flag = entryLine[17];
            if (flag != 'n' && flag != 'f')
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::corrupted_xref_table)
                                           .with_component(error_component::cross_reference)
                                           .with_field("entry_in_use_flag")
                                           .with_expected("n or f")
                                           .with_actual(std::string{1, flag})
                                           .with_message("Invalid xref in-use flag")
                                           .build());
            }

            const bool inUse = (flag == 'n');

            const std::uint32_t objectNumber = static_cast<std::uint32_t>(*startObj + i);
            const indirect_reference ref{objectNumber, generation};

            entries.insert_or_assign(objectNumber, cross_reference_entry{ref, offset, inUse});
        }

        return {};
    }

    std::expected<cross_reference_table, error> default_cross_reference_table_parser::parse(
        std::string_view content)
    {
        cross_reference_table::entry_map entries;

        const std::size_t firstNewline = content.find('\n');
        if (firstNewline == std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unexpected_eof)
                                       .with_component(error_component::cross_reference)
                                       .with_field("xref")
                                       .with_message("Unexpected EOF while parsing xref")
                                       .build());
        }

        std::string_view xrefLine = content.substr(0, firstNewline);
        xrefLine = text::trim_ascii(text::strip_line_endings(xrefLine));
        if (!text::starts_with_token(xrefLine, "xref"))
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_xref_table)
                                       .with_component(error_component::cross_reference)
                                       .with_field("xref_keyword")
                                       .with_expected("xref")
                                       .with_actual(std::string{xrefLine})
                                       .with_message("Missing xref keyword")
                                       .build());
        }

        content = content.substr(firstNewline + 1);

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

        return cross_reference_table{std::move(entries)};
    }
}
