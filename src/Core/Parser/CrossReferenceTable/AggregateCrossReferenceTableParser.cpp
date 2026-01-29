#include "Core/Parser/CrossReferenceTable/AggregateCrossReferenceTableParser.hpp"

#include <algorithm>
#include <array>

#include "Core/Parser/CrossReferenceTable/DefaultCrossReferenceTableParser.hpp"
#include "Core/Reader/SubReader.hpp"
#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    AggregateCrossReferenceTableParser::AggregateCrossReferenceTableParser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<std::size_t, ParserError> AggregateCrossReferenceTableParser::FindStartXrefOffset()
    {
        constexpr std::string_view kStartXrefKeyword = "startxref";
        constexpr std::size_t kLineBufferSize = 256;
        constexpr std::size_t kSearchAreaSize = 1024;

        const std::uint64_t fileSize = _reader.Size();
        const std::size_t searchPos = fileSize > kSearchAreaSize ? fileSize - kSearchAreaSize : 0;

        _reader.Seek(searchPos);

        std::array<std::byte, kLineBufferSize> buffer{};
        bool foundKeyword = false;

        while (!_reader.Eof())
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                break;
            }

            const std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            if (Text::StartsWithToken(line, kStartXrefKeyword))
            {
                foundKeyword = true;
                break;
            }
        }

        if (!foundKeyword)
        {
            return std::unexpected(ParserError::MissingCrossReferenceTable);
        }

        const std::size_t bytesRead = _reader.ReadLine(buffer);
        if (bytesRead == 0)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        const std::string_view offsetLine{
            reinterpret_cast<const char *>(buffer.data()),
            bytesRead};

        const auto offset = Text::ParseSizeT(offsetLine);
        if (!offset)
        {
            return std::unexpected(ParserError::CorruptedCrossReferenceTable);
        }

        return *offset;
    }

    std::expected<std::size_t, ParserError> AggregateCrossReferenceTableParser::FindPrevXrefOffset(std::size_t currentXrefOffset)
    {
        constexpr std::size_t kLineBufferSize = 256;
        std::array<std::byte, kLineBufferSize> buffer{};

        _reader.Seek(currentXrefOffset);

        bool foundTrailer = false;
        for (std::size_t i = 0; i < 100; ++i)
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            const std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            if (Text::StartsWithToken(line, "trailer"))
            {
                foundTrailer = true;
                break;
            }
        }

        if (!foundTrailer)
        {
            return std::unexpected(ParserError::CorruptedCrossReferenceTable);
        }

        for (std::size_t i = 0; i < 20; ++i)
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                break;
            }

            const std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            const std::size_t prevPos = line.find("/Prev");
            if (prevPos != std::string_view::npos)
            {
                std::string_view rest = line.substr(prevPos + 5);
                rest = Text::TrimAscii(rest);

                const auto parsed = Text::ParseSizeT(rest);
                if (parsed)
                {
                    return *parsed;
                }
            }

            if (Text::StartsWithToken(line, "startxref") || line.find("%%EOF") != std::string_view::npos)
            {
                break;
            }
        }

        return std::unexpected(ParserError::MissingCrossReferenceTable);
    }

    std::expected<std::vector<std::size_t>, ParserError> AggregateCrossReferenceTableParser::LocateAllXrefOffsets()
    {
        auto startXrefResult = FindStartXrefOffset();
        if (!startXrefResult)
        {
            return std::unexpected(startXrefResult.error());
        }

        std::vector<std::size_t> offsets;
        std::vector<std::size_t> visitedOffsets;

        std::size_t currentOffset = startXrefResult.value();

        while (true)
        {
            if (std::find(visitedOffsets.begin(), visitedOffsets.end(), currentOffset) != visitedOffsets.end())
            {
                break;
            }

            offsets.push_back(currentOffset);
            visitedOffsets.push_back(currentOffset);

            auto prevResult = FindPrevXrefOffset(currentOffset);
            if (!prevResult)
            {
                break;
            }

            currentOffset = *prevResult;
        }

        return offsets;
    }

    std::unique_ptr<CrossReferenceTableParser> AggregateCrossReferenceTableParser::CreateParserForOffset(std::size_t offset)
    {
        // For now, we only support traditional xref tables
        // In the future, this could detect compressed xref streams and return appropriate parser
        auto subReader = std::make_unique<SubReader>(_reader, offset);

        return std::make_unique<DefaultCrossReferenceTableParser>(*subReader);
    }

    std::expected<CrossReferenceTableParseResult, ParserError> AggregateCrossReferenceTableParser::Parse()
    {
        auto offsetsResult = LocateAllXrefOffsets();
        if (!offsetsResult)
        {
            return std::unexpected(offsetsResult.error());
        }

        const auto &offsets = offsetsResult.value();
        if (offsets.empty())
        {
            return std::unexpected(ParserError::MissingCrossReferenceTable);
        }

        CrossReferenceTable mergedTable;
        std::vector<Breakpoint> allBreakpoints;

        for (std::size_t offset : offsets)
        {
            _reader.Seek(offset);

            DefaultCrossReferenceTableParser parser{_reader};
            auto parseResult = parser.Parse();
            if (!parseResult)
            {
                return std::unexpected(parseResult.error());
            }

            for (const auto &[objectNum, entry] : parseResult->table.Entries())
            {
                mergedTable.AddEntry(objectNum, entry);
            }

            allBreakpoints.insert(allBreakpoints.end(),
                parseResult->breakpoints.begin(),
                parseResult->breakpoints.end());
        }

        return CrossReferenceTableParseResult{
            .table = std::move(mergedTable),
            .breakpoints = std::move(allBreakpoints)
        };
    }
}
