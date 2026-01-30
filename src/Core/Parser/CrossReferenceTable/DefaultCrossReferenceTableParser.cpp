#include "Core/Parser/CrossReferenceTable/DefaultCrossReferenceTableParser.hpp"

#include <charconv>
#include <string_view>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    std::expected<void, ParserError> DefaultCrossReferenceTableParser::ParseSubsection(
        CrossReferenceTable &table,
        std::string_view &content)
    {
        // Find next newline to get header line
        const std::size_t newlinePos = content.find('\n');
        if (newlinePos == std::string_view::npos)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        std::string_view headerLine = content.substr(0, newlinePos);
        headerLine = Text::TrimAscii(Text::StripLineEndings(headerLine));
        content = content.substr(newlinePos + 1);

        // Parse start object number and count
        const std::size_t spacePos = headerLine.find(' ');
        if (spacePos == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedCrossReferenceTable);
        }

        const auto startObj = Text::ParseSizeT(headerLine.substr(0, spacePos));
        const auto count = Text::ParseSizeT(headerLine.substr(spacePos + 1));

        if (!startObj || !count)
        {
            return std::unexpected(ParserError::CorruptedCrossReferenceTable);
        }

        // Parse entries
        for (std::size_t i = 0; i < *count; ++i)
        {
            const std::size_t entryNewline = content.find('\n');
            if (entryNewline == std::string_view::npos)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view entryLine = content.substr(0, entryNewline);
            entryLine = Text::TrimAscii(Text::StripLineEndings(entryLine));
            content = content.substr(entryNewline + 1);

            if (entryLine.size() < 18)
            {
                return std::unexpected(ParserError::CorruptedCrossReferenceTable);
            }

            // Parse offset (10 digits)
            std::uint64_t offset = 0;
            auto [ptr1, ec1] = std::from_chars(
                entryLine.data(),
                entryLine.data() + 10,
                offset);

            if (ec1 != std::errc{})
            {
                return std::unexpected(ParserError::CorruptedCrossReferenceTable);
            }

            // Parse generation (5 digits, starts at position 11)
            std::uint16_t generation = 0;
            auto [ptr2, ec2] = std::from_chars(
                entryLine.data() + 11,
                entryLine.data() + 16,
                generation);

            if (ec2 != std::errc{})
            {
                return std::unexpected(ParserError::CorruptedCrossReferenceTable);
            }

            // Parse in-use flag (position 17)
            const char flag = entryLine[17];
            const bool inUse = (flag == 'n');

            const std::uint32_t objectNumber = static_cast<std::uint32_t>(*startObj + i);
            table.AddEntry(objectNumber, CrossReferenceEntry{offset, generation, inUse});
        }

        return {};
    }

    std::expected<CrossReferenceTableParseResult, ParserError> DefaultCrossReferenceTableParser::Parse(
        std::string_view content)
    {
        CrossReferenceTable table;

        // Find first newline to get xref keyword line
        const std::size_t firstNewline = content.find('\n');
        if (firstNewline == std::string_view::npos)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        const std::string_view xrefLine = content.substr(0, firstNewline);
        if (!Text::StartsWithToken(xrefLine, "xref"))
        {
            return std::unexpected(ParserError::MissingCrossReferenceTable);
        }

        content = content.substr(firstNewline + 1);

        // Parse subsections until we hit "trailer"
        while (!content.empty())
        {
            // Check if we've reached the trailer
            if (Text::StartsWithToken(content, "trailer"))
            {
                break;
            }

            auto result = ParseSubsection(table, content);
            if (!result)
            {
                return std::unexpected(result.error());
            }
        }

        return CrossReferenceTableParseResult{
            .table = std::move(table)
        };
    }
}
