#include "Core/Parser/CrossReferenceTable/CrossReferenceTableLocator.hpp"

#include <array>
#include <algorithm>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    CrossReferenceTableLocator::CrossReferenceTableLocator(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<std::size_t, ParserError> CrossReferenceTableLocator::FindStartXrefOffset()
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

    std::expected<std::size_t, ParserError> CrossReferenceTableLocator::FindPrevXrefOffset(std::size_t currentXrefOffset)
    {
        constexpr std::size_t kLineBufferSize = 256;
        std::array<std::byte, kLineBufferSize> buffer{};

        _reader.Seek(currentXrefOffset);

        // Skip to trailer section
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

        // Read trailer dictionary to find /Prev
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

    std::expected<std::vector<std::size_t>, ParserError> CrossReferenceTableLocator::FindAllXrefOffsets()
    {
        auto startXrefResult = FindStartXrefOffset();
        if (!startXrefResult)
        {
            return std::unexpected(startXrefResult.error());
        }

        std::vector<std::size_t> offsets;
        std::vector<std::size_t> visitedOffsets;

        std::size_t currentOffset = *startXrefResult;

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
}
