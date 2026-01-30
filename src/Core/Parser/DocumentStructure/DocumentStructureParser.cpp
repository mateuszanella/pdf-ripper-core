#include "Core/Parser/DocumentStructure/DocumentStructureParser.hpp"

#include <algorithm>
#include <array>
#include <unordered_set>

#include "Core/Parser/CrossReferenceTable/DefaultCrossReferenceTableParser.hpp"
#include "Core/Parser/Trailer/DefaultTrailerParser.hpp"
#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    DocumentStructureParser::DocumentStructureParser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<std::size_t, ParserError> DocumentStructureParser::FindStartXrefOffset()
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

    std::expected<std::size_t, ParserError> DocumentStructureParser::ExtractPrevOffset(const Trailer &trailer)
    {
        if (!trailer.Prev())
        {
            return std::unexpected(ParserError::MissingCrossReferenceTable);
        }

        return *trailer.Prev();
    }

    std::expected<DocumentStructureResult, ParserError> DocumentStructureParser::Parse()
    {
        // Step 1: Find startxref
        auto startXrefResult = FindStartXrefOffset();
        if (!startXrefResult)
        {
            return std::unexpected(startXrefResult.error());
        }

        DocumentStructureResult result;
        std::unordered_set<std::size_t> visitedOffsets;
        std::size_t currentOffset = *startXrefResult;

        while (true)
        {
            if (visitedOffsets.contains(currentOffset))
            {
                break;
            }
            visitedOffsets.insert(currentOffset);

            // Step 2: Collect xref and trailer bytes
            _reader.Seek(currentOffset);

            // Collect bytes until we find the end of trailer (>>)
            std::string collectedContent;
            constexpr std::size_t kBufferSize = 4096;
            std::array<std::byte, kBufferSize> buffer{};
            bool foundTrailerEnd = false;

            while (!_reader.Eof() && !foundTrailerEnd)
            {
                const std::size_t bytesRead = _reader.Read(buffer);
                if (bytesRead == 0)
                {
                    break;
                }

                std::string_view chunk{
                    reinterpret_cast<const char *>(buffer.data()),
                    bytesRead};
                collectedContent += chunk;

                // Check if we've collected the complete trailer
                if (collectedContent.find("trailer") != std::string::npos &&
                    collectedContent.find(">>") != std::string::npos)
                {
                    foundTrailerEnd = true;
                }
            }

            if (!foundTrailerEnd)
            {
                if (result.xrefTableHistory.empty())
                {
                    return std::unexpected(ParserError::MissingTrailer);
                }
                break;
            }

            // Step 3: Parse xref from collected bytes
            DefaultCrossReferenceTableParser xrefParser;
            auto xrefResult = xrefParser.Parse(collectedContent);
            if (!xrefResult)
            {
                if (result.xrefTableHistory.empty())
                {
                    return std::unexpected(xrefResult.error());
                }
                break;
            }

            result.xrefTableHistory.push_back(std::move(xrefResult->table));

            // Step 5: Parse trailer from collected bytes
            DefaultTrailerParser trailerParser;
            auto trailerResult = trailerParser.Parse(collectedContent);
            if (!trailerResult)
            {
                if (result.trailerHistory.empty())
                {
                    return std::unexpected(trailerResult.error());
                }
                break;
            }

            result.trailerHistory.push_back(std::move(trailerResult.value()));

            // Step 6: Check for /Prev to repeat
            auto prevOffsetResult = ExtractPrevOffset(result.trailerHistory.back());
            if (!prevOffsetResult)
            {
                break;
            }

            currentOffset = *prevOffsetResult;
        }

        // Compile merged view (newer entries override older ones)
        for (auto it = result.xrefTableHistory.rbegin(); it != result.xrefTableHistory.rend(); ++it)
        {
            for (const auto &[objectNum, entry] : it->Entries())
            {
                result.compiledXrefTable.AddEntry(objectNum, entry);
            }
        }

        for (auto it = result.trailerHistory.rbegin(); it != result.trailerHistory.rend(); ++it)
        {
            result.compiledTrailer.Merge(*it);
        }

        return result;
    }
}
