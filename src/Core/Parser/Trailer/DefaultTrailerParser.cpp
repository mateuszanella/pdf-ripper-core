#include "Core/Parser/Trailer/DefaultTrailerParser.hpp"

#include <array>
#include <charconv>
#include <string_view>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    DefaultTrailerParser::DefaultTrailerParser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<std::pair<std::uint32_t, std::uint16_t>, ParserError>
        DefaultTrailerParser::ParseIndirectReference(std::string_view line)
    {
        // Parse "objNum genNum R"
        const std::size_t firstSpace = line.find(' ');
        if (firstSpace == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        const auto objNum = Text::ParseSizeT(line.substr(0, firstSpace));
        if (!objNum)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        line = line.substr(firstSpace + 1);
        line = Text::TrimAscii(line);

        const std::size_t secondSpace = line.find(' ');
        if (secondSpace == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        const auto genNum = Text::ParseSizeT(line.substr(0, secondSpace));
        if (!genNum)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        return std::make_pair(
            static_cast<std::uint32_t>(*objNum),
            static_cast<std::uint16_t>(*genNum)
        );
    }

    std::expected<Trailer, ParserError> DefaultTrailerParser::ParseDictionary(
        std::string_view content)
    {
        Trailer trailer;

        // Parse /Size
        if (const std::size_t sizePos = content.find("/Size"); sizePos != std::string_view::npos)
        {
            std::string_view rest = content.substr(sizePos + 5);
            rest = Text::TrimAscii(rest);

            const auto size = Text::ParseSizeT(rest);
            if (size)
            {
                trailer.SetSize(static_cast<std::uint32_t>(*size));
            }
        }

        // Parse /Prev
        if (const std::size_t prevPos = content.find("/Prev"); prevPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(prevPos + 5);
            rest = Text::TrimAscii(rest);

            const auto prev = Text::ParseSizeT(rest);
            if (prev)
            {
                trailer.SetPrev(*prev);
            }
        }

        // Parse /Root
        if (const std::size_t rootPos = content.find("/Root"); rootPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(rootPos + 5);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetRoot(ref->first, ref->second);
            }
        }

        // Parse /Info
        if (const std::size_t infoPos = content.find("/Info"); infoPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(infoPos + 5);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetInfo(ref->first, ref->second);
            }
        }

        // Parse /Encrypt
        if (const std::size_t encryptPos = content.find("/Encrypt"); encryptPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(encryptPos + 8);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetEncrypt(ref->first, ref->second);
            }
        }

        // Parse /ID
        if (const std::size_t idPos = content.find("/ID"); idPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(idPos + 3);
            rest = Text::TrimAscii(rest);

            if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
            {
                if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                {
                    std::string idValue{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                    trailer.SetID(idValue);
                }
            }
        }

        return trailer;
    }

    std::expected<Trailer, ParserError> DefaultTrailerParser::Parse()
    {
        constexpr std::size_t kLineBufferSize = 512;
        std::array<std::byte, kLineBufferSize> buffer{};

        // Find "trailer" keyword
        bool foundKeyword = false;

        while (!_reader.Eof())
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
                foundKeyword = true;
                break;
            }
        }

        if (!foundKeyword)
        {
            return std::unexpected(ParserError::MissingTrailer);
        }

        // Read and accumulate dictionary content until we find both << and >>
        std::string accumulatedContent;
        bool foundStart = false;
        bool foundEnd = false;

        while (!_reader.Eof() && !foundEnd)
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            accumulatedContent += std::string(line) + " ";

            if (!foundStart && accumulatedContent.find("<<") != std::string::npos)
            {
                foundStart = true;
            }

            if (foundStart && accumulatedContent.find(">>") != std::string::npos)
            {
                foundEnd = true;
            }
        }

        if (!foundStart || !foundEnd)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        // Extract content between << and >>
        const std::size_t startPos = accumulatedContent.find("<<");
        const std::size_t endPos = accumulatedContent.find(">>");

        if (startPos == std::string::npos || endPos == std::string::npos || endPos <= startPos)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        std::string_view dictContent = std::string_view(accumulatedContent).substr(
            startPos + 2,
            endPos - startPos - 2
        );

        std::printf("Trailer Dictionary Content: %.*s\n",
                    static_cast<int>(dictContent.size()),
                    dictContent.data());

        // Parse the dictionary
        auto trailerResult = ParseDictionary(dictContent);
        if (!trailerResult)
        {
            return std::unexpected(trailerResult.error());
        }

        return std::move(trailerResult.value());
    }
}
