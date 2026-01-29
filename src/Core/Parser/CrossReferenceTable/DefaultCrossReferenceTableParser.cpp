#include "Core/Parser/CrossReferenceTable/DefaultCrossReferenceTableParser.hpp"

#include <array>
#include <charconv>
#include <string_view>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    DefaultCrossReferenceTableParser::DefaultCrossReferenceTableParser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<void, ParserError> DefaultCrossReferenceTableParser::ParseSubsection(
        CrossReferenceTable &table,
        std::vector<Breakpoint> &breakpoints)
    {
        constexpr std::size_t kLineBufferSize = 256;
        constexpr std::size_t kXrefEntryLength = 20; // "nnnnnnnnnn ggggg n/f \n"

        std::array<std::byte, kLineBufferSize> buffer{};

        // Read subsection header: "startObj count"
        std::size_t bytesRead = _reader.ReadLine(buffer);
        if (bytesRead == 0)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        std::string_view headerLine{
            reinterpret_cast<const char *>(buffer.data()),
            bytesRead};
        headerLine = Text::TrimAscii(Text::StripLineEndings(headerLine));

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

        const std::size_t subsectionStart = _reader.Tell();
        breakpoints.emplace_back(subsectionStart, BreakpointType::XrefStart);

        // Parse entries
        for (std::size_t i = 0; i < *count; ++i)
        {
            bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view entryLine{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};
            entryLine = Text::TrimAscii(Text::StripLineEndings(entryLine));

            if (entryLine.size() < 18) // Minimum: "0000000000 00000 n"
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
            if (entryLine.size() <= 17)
            {
                return std::unexpected(ParserError::CorruptedCrossReferenceTable);
            }

            const char flag = entryLine[17];
            const bool inUse = (flag == 'n');

            const std::uint32_t objectNumber = static_cast<std::uint32_t>(*startObj + i);
            table.AddEntry(objectNumber, CrossReferenceEntry{offset, generation, inUse});
        }

        const std::size_t subsectionEnd = _reader.Tell();
        breakpoints.emplace_back(subsectionEnd, BreakpointType::XrefEnd);

        return {};
    }

    std::expected<CrossReferenceTableParseResult, ParserError> DefaultCrossReferenceTableParser::Parse()
    {
        constexpr std::size_t kLineBufferSize = 256;

        CrossReferenceTable table;
        std::vector<Breakpoint> breakpoints;
        breakpoints.reserve(10);

        std::array<std::byte, kLineBufferSize> buffer{};

        const std::size_t xrefKeywordPos = _reader.Tell();
        std::size_t bytesRead = _reader.ReadLine(buffer);
        if (bytesRead == 0)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        const std::string_view xrefLine{
            reinterpret_cast<const char *>(buffer.data()),
            bytesRead};

        if (!Text::StartsWithToken(xrefLine, "xref"))
        {
            return std::unexpected(ParserError::MissingCrossReferenceTable);
        }

        breakpoints.emplace_back(xrefKeywordPos, BreakpointType::XrefKeyword);

        while (true)
        {
            const std::size_t lineStart = _reader.Tell();
            bytesRead = _reader.ReadLine(buffer);

            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            if (Text::StartsWithToken(line, "trailer"))
            {
                breakpoints.emplace_back(lineStart, BreakpointType::TrailerKeyword);
                break;
            }

            _reader.Seek(lineStart);
            auto result = ParseSubsection(table, breakpoints);
            if (!result)
            {
                return std::unexpected(result.error());
            }
        }

        return CrossReferenceTableParseResult{
            .table = std::move(table),
            .breakpoints = std::move(breakpoints)
        };
    }
}
