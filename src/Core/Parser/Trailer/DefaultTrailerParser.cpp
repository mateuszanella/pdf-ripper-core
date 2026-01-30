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

    std::expected<void, ParserError> DefaultTrailerParser::ParseDictionary(
        Trailer &trailer)
    {
        constexpr std::size_t kLineBufferSize = 512;
        std::array<std::byte, kLineBufferSize> buffer{};

        // Expect dictionary start "<<" on next line after "trailer"
        std::size_t bytesRead = _reader.ReadLine(buffer);
        if (bytesRead == 0)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        std::string_view line{
            reinterpret_cast<const char *>(buffer.data()),
            bytesRead};

        if (line.find("<<") == std::string_view::npos)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        // Parse dictionary entries until we find >>
        std::string accumulatedLine;

        for (std::size_t i = 0; i < 50; ++i)
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            accumulatedLine += std::string(line);

            // Check if we found the end
            if (accumulatedLine.find(">>") != std::string::npos)
            {
                break;
            }
        }

        // Now parse the accumulated dictionary content
        std::string_view dictContent = accumulatedLine;

        std::printf("Trailer Dictionary Content: %s\n", dictContent.data());

        // Parse /Size
        if (const std::size_t sizePos = dictContent.find("/Size"); sizePos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(sizePos + 5);
            rest = Text::TrimAscii(rest);

            const auto size = Text::ParseSizeT(rest);
            if (size)
            {
                trailer.SetSize(static_cast<std::uint32_t>(*size));
            }
        }

        // Parse /Prev
        if (const std::size_t prevPos = dictContent.find("/Prev"); prevPos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(prevPos + 5);
            rest = Text::TrimAscii(rest);

            const auto prev = Text::ParseSizeT(rest);
            if (prev)
            {
                trailer.SetPrev(*prev);
            }
        }

        // Parse /Root
        if (const std::size_t rootPos = dictContent.find("/Root"); rootPos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(rootPos + 5);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetRoot(ref->first, ref->second);
            }
        }

        // Parse /Info
        if (const std::size_t infoPos = dictContent.find("/Info"); infoPos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(infoPos + 5);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetInfo(ref->first, ref->second);
            }
        }

        // Parse /Encrypt
        if (const std::size_t encryptPos = dictContent.find("/Encrypt"); encryptPos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(encryptPos + 8);
            rest = Text::TrimAscii(rest);

            auto ref = ParseIndirectReference(rest);
            if (ref)
            {
                trailer.SetEncrypt(ref->first, ref->second);
            }
        }

        // Parse /ID
        if (const std::size_t idPos = dictContent.find("/ID"); idPos != std::string_view::npos)
        {
            std::string_view rest = dictContent.substr(idPos + 3);
            rest = Text::TrimAscii(rest);

            // Simple extraction of ID (just get the hex string content for now)
            if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
            {
                if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                {
                    std::string idValue{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                    trailer.SetID(idValue);
                }
            }
        }

        return {};
    }

    std::expected<TrailerParseResult, ParserError> DefaultTrailerParser::Parse()
    {
        constexpr std::size_t kLineBufferSize = 256;
        std::array<std::byte, kLineBufferSize> buffer{};

        Trailer trailer;

        // Find "trailer" keyword
        bool foundKeyword = false;

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
                foundKeyword = true;
                break;
            }
        }

        if (!foundKeyword)
        {
            return std::unexpected(ParserError::MissingTrailer);
        }

        // Parse dictionary
        auto result = ParseDictionary(trailer);
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return TrailerParseResult{
            .trailer = std::move(trailer)
        };
    }
}
