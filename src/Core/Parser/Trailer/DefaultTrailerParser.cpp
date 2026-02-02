#include "Core/Parser/Trailer/DefaultTrailerParser.hpp"

#include <string_view>

#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
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

            // Look for array opening bracket
            if (const std::size_t arrayStart = rest.find('['); arrayStart != std::string_view::npos)
            {
                rest = rest.substr(arrayStart + 1);

                std::string firstId;
                std::optional<std::string> secondId;

                // Parse first hex string (required)
                rest = Text::TrimAscii(rest);
                if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
                {
                    if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                    {
                        firstId = std::string{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                        rest = rest.substr(hexEnd + 1);
                    }
                }

                // Parse second hex string (optional)
                rest = Text::TrimAscii(rest);
                if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
                {
                    if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                    {
                        secondId = std::string{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                    }
                }

                if (!firstId.empty())
                {
                    trailer.SetID({firstId, secondId});
                }
            }
        }

        return trailer;
    }

    std::expected<Trailer, ParserError> DefaultTrailerParser::Parse(std::string_view content)
    {
        // Find "trailer" keyword
        const std::size_t trailerPos = content.find("trailer");
        if (trailerPos == std::string_view::npos)
        {
            return std::unexpected(ParserError::MissingTrailer);
        }

        content = content.substr(trailerPos + 7); // Skip "trailer"

        // Find dictionary delimiters
        const std::size_t startPos = content.find("<<");
        const std::size_t endPos = content.find(">>");

        if (startPos == std::string_view::npos || endPos == std::string_view::npos || endPos <= startPos)
        {
            return std::unexpected(ParserError::CorruptedTrailer);
        }

        std::string_view dictContent = content.substr(startPos + 2, endPos - startPos - 2);

        return ParseDictionary(dictContent);
    }
}
