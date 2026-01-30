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
        auto startXrefResult = FindStartXrefOffset();
        if (!startXrefResult)
        {
            return std::unexpected(startXrefResult.error());
        }

        DocumentStructureResult result;
        std::unordered_set<std::size_t> visitedOffsets;
        std::size_t currentOffset = *startXrefResult;

        // Parse xref/trailer pairs following the chain
        while (true)
        {
            if (visitedOffsets.contains(currentOffset))
            {
                break;
            }
            visitedOffsets.insert(currentOffset);

            // Seek to xref table position
            _reader.Seek(currentOffset);

            // Parse xref table
            DefaultCrossReferenceTableParser xrefParser{_reader};
            auto xrefResult = xrefParser.Parse();
            if (!xrefResult)
            {
                // If this isn't the first table, we can tolerate failure
                if (result.xrefTableHistory.empty())
                {
                    return std::unexpected(xrefResult.error());
                }

                break;
            }

            result.xrefTableHistory.push_back(xrefResult->table);

            // Parse trailer (reader should already be positioned after xref)
            DefaultTrailerParser trailerParser{_reader};
            auto trailerResult = trailerParser.Parse();
            if (!trailerResult)
            {
                // If this isn't the first trailer, we can tolerate failure
                if (result.trailerHistory.empty())
                {
                    return std::unexpected(trailerResult.error());
                }

                break;
            }

            result.trailerHistory.push_back(trailerResult->trailer);

            // Check for /Prev entry to continue chain
            auto prevOffsetResult = ExtractPrevOffset(trailerResult->trailer);
            if (!prevOffsetResult)
            {
                break;
            }

            currentOffset = *prevOffsetResult;
        }

        // Compile merged view (newer entries override older ones)
        // History is in newest-to-oldest order, so merge in reverse
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
