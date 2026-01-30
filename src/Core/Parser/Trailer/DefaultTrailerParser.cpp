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
        Trailer &trailer,
        std::vector<Breakpoint> &breakpoints)
    {
        constexpr std::size_t kLineBufferSize = 512;
        std::array<std::byte, kLineBufferSize> buffer{};

        const std::size_t dictStart = _reader.Tell();
        bool foundDictStart = false;
        bool foundDictEnd = false;

        // Find dictionary start "<<"
        for (std::size_t i = 0; i < 10; ++i)
        {
            const std::size_t bytesRead = _reader.ReadLine(buffer);
            if (bytesRead == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            if (line.find("<<") != std::string_view::npos)
            {
                foundDictStart = true;
                breakpoints.emplace_back(_reader.Tell() - bytesRead, BreakpointType::TrailerStart);
                break;
            }
        }

        if (!foundDictStart)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        // Parse dictionary entries
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
            line = Text::TrimAscii(line);

            if (line.find(">>") != std::string_view::npos)
            {
                foundDictEnd = true;
                breakpoints.emplace_back(_reader.Tell(), BreakpointType::TrailerEnd);
                break;
            }

            // Parse /Size
            if (line.find("/Size") != std::string_view::npos)
            {
                const std::size_t pos = line.find("/Size");
                std::string_view rest = line.substr(pos + 5);
                rest = Text::TrimAscii(rest);

                const auto size = Text::ParseSizeT(rest);
                if (size)
                {
                    trailer.SetSize(static_cast<std::uint32_t>(*size));
                }
            }

            // Parse /Prev
            if (line.find("/Prev") != std::string_view::npos)
            {
                const std::size_t pos = line.find("/Prev");
                std::string_view rest = line.substr(pos + 5);
                rest = Text::TrimAscii(rest);

                const auto prev = Text::ParseSizeT(rest);
                if (prev)
                {
                    trailer.SetPrev(*prev);
                }
            }

            // Parse /Root
            if (line.find("/Root") != std::string_view::npos)
            {
                const std::size_t pos = line.find("/Root");
                std::string_view rest = line.substr(pos + 5);
                rest = Text::TrimAscii(rest);

                auto ref = ParseIndirectReference(rest);
                if (ref)
                {
                    trailer.SetRoot(ref->first, ref->second);
                }
            }

            // Parse /Info
            if (line.find("/Info") != std::string_view::npos)
            {
                const std::size_t pos = line.find("/Info");
                std::string_view rest = line.substr(pos + 5);
                rest = Text::TrimAscii(rest);

                auto ref = ParseIndirectReference(rest);
                if (ref)
                {
                    trailer.SetInfo(ref->first, ref->second);
                }
            }

            // Parse /Encrypt
            if (line.find("/Encrypt") != std::string_view::npos)
            {
                const std::size_t pos = line.find("/Encrypt");
                std::string_view rest = line.substr(pos + 8);
                rest = Text::TrimAscii(rest);

                auto ref = ParseIndirectReference(rest);
                if (ref)
                {
                    trailer.SetEncrypt(ref->first, ref->second);
                }
            }
        }

        if (!foundDictEnd)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        return {};
    }

    std::expected<TrailerParseResult, ParserError> DefaultTrailerParser::Parse()
    {
        constexpr std::size_t kLineBufferSize = 256;
        std::array<std::byte, kLineBufferSize> buffer{};

        Trailer trailer;
        std::vector<Breakpoint> breakpoints;
        breakpoints.reserve(5);

        // Find "trailer" keyword
        bool foundKeyword = false;
        std::size_t keywordPos = 0;

        for (std::size_t i = 0; i < 20; ++i)
        {
            keywordPos = _reader.Tell();
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
                breakpoints.emplace_back(keywordPos, BreakpointType::TrailerKeyword);
                break;
            }
        }

        if (!foundKeyword)
        {
            return std::unexpected(ParserError::MissingTrailer);
        }

        // Parse dictionary
        auto result = ParseDictionary(trailer, breakpoints);
        if (!result)
        {
            return std::unexpected(result.error());
        }

        return TrailerParseResult{
            .trailer = std::move(trailer),
            .breakpoints = std::move(breakpoints)
        };
    }
}
