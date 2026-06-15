#include "ripper/pdf/core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/util/text.hpp"

#include <charconv>
#include <string>
#include <string_view>

namespace ripper::pdf::core
{
void default_cross_reference_table_parser::parse_subsection(cross_reference_section& section,
                                                            std::string_view& content)
{
    const std::size_t newlinePos = content.find('\n');
    if (newlinePos == std::string_view::npos)
    {
        throw parse_exception{"Unexpected EOF while parsing xref subsection header"};
    }

    std::string_view headerLine = content.substr(0, newlinePos);
    headerLine = text::trim_ascii(text::strip_line_endings(headerLine));
    content = content.substr(newlinePos + 1);

    const std::size_t spacePos = headerLine.find(' ');
    if (spacePos == std::string_view::npos)
    {
        throw parse_exception{"Invalid xref subsection header"};
    }

    const auto startObj = text::parse_size_t(headerLine.substr(0, spacePos));
    const auto count = text::parse_size_t(headerLine.substr(spacePos + 1));

    if (!startObj || !count)
    {
        throw parse_exception{"Invalid xref subsection range"};
    }

    for (std::size_t i = 0; i < *count; ++i)
    {
        const std::size_t entryNewline = content.find('\n');
        if (entryNewline == std::string_view::npos)
        {
            throw parse_exception{"Unexpected EOF while parsing xref entries"};
        }

        std::string_view entryLine = content.substr(0, entryNewline);
        entryLine = text::trim_ascii(text::strip_line_endings(entryLine));
        content = content.substr(entryNewline + 1);

        if (entryLine.size() < 18)
        {
            throw parse_exception{"Malformed xref entry"};
        }

        std::uint64_t offset = 0;
        // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage,
        // cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr1, ec1] = std::from_chars(entryLine.data(), entryLine.data() + 10, offset);

        if (ec1 != std::errc{})
        {
            throw parse_exception{"Invalid xref entry offset"};
        }

        std::uint16_t generation = 0;
        auto [ptr2, ec2] =
            std::from_chars(entryLine.data() + 11, entryLine.data() + 16,
                            generation); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        if (ec2 != std::errc{})
        {
            throw parse_exception{"Invalid xref entry generation"};
        }

        const char flag = entryLine[17];
        if (flag != 'n' && flag != 'f')
        {
            throw parse_exception{"Invalid xref in-use flag"};
        }

        const bool inUse = (flag == 'n');

        const std::uint32_t objectNumber = static_cast<std::uint32_t>(*startObj + i);
        const indirect_reference ref{objectNumber, generation};

        section.add_entry(cross_reference_entry{ref, offset, inUse});
    }
}

cross_reference_section default_cross_reference_table_parser::parse(std::string_view content)
{
    const std::size_t firstNewline = content.find('\n');
    if (firstNewline == std::string_view::npos)
    {
        throw parse_exception{"Unexpected EOF while parsing xref"};
    }

    std::string_view xrefLine = content.substr(0, firstNewline);
    xrefLine = text::trim_ascii(text::strip_line_endings(xrefLine));
    if (!text::starts_with_token(xrefLine, "xref"))
    {
        throw parse_exception{"Missing xref keyword"};
    }

    content = content.substr(firstNewline + 1);

    cross_reference_section section{std::vector<cross_reference_subsection>{}};

    while (!content.empty())
    {
        content = text::trim_ascii(content);
        if (content.empty() || text::starts_with_token(content, "trailer"))
        {
            break;
        }

        parse_subsection(section, content);
    }

    return section;
}
} // namespace ripper::pdf::core
