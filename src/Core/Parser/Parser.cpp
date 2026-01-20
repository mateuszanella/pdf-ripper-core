#include "Core/Parser/Parser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string_view>
#include <stdexcept>

#include "Core/Reader/Reader.hpp"
#include "Core/Util/Text.hpp"

namespace Ripper::Core
{
    Parser::Parser(Reader &reader)
        : _reader{reader}
    {
    }

    /**
     * @todo this should be delegated to another class
     */
    std::expected<std::string, ParserError> Parser::ReadHeader()
    {
        constexpr std::string_view kMagic = "%PDF-";
        constexpr std::size_t kMaxHeaderLineLength = 256;

        std::array<std::byte, kMaxHeaderLineLength> buffer{};

        _reader.Seek(0);

        const std::size_t read = _reader.ReadLine(buffer);
        if (read == 0)
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        const std::string_view line{
            reinterpret_cast<const char *>(buffer.data()),
            read};

        const std::size_t pos = line.find(kMagic);
        if (pos == std::string_view::npos)
        {
            return std::unexpected(ParserError::MissingHeader);
        }

        const std::string_view rest = line.substr(pos + kMagic.size());

        std::size_t len = 0;
        while (len < rest.size())
        {
            const unsigned char ch = static_cast<unsigned char>(rest[len]);
            if (!(std::isdigit(ch) || ch == '.'))
                break;
            ++len;
        }

        if (len == 0)
        {
            return std::unexpected(ParserError::CorruptedHeader);
        }

        return std::string{rest.substr(0, len)};
    }

    std::expected<std::string, ParserError> Parser::ReadCrossReferenceTable()
    {
        constexpr std::size_t kMaxLineLength = 8192;
        std::array<std::byte, kMaxLineLength> buffer{};

        _reader.Seek(0);

        std::optional<std::size_t> lastXrefOffset;
        bool awaitingOffsetLine = false;

        while (true)
        {
            const std::size_t read = _reader.ReadLine(buffer);
            if (read == 0)
                break;

            const std::string_view rawLine{
                reinterpret_cast<const char *>(buffer.data()),
                read};

            const std::string_view line = Text::TrimAscii(Text::StripLineEndings(rawLine));

            // Typical layout:
            // startxref
            // 12345
            // %%EOF
            if (awaitingOffsetLine)
            {
                if (auto off = Text::ParseSizeT(line))
                    lastXrefOffset = *off;
                awaitingOffsetLine = false;
                continue;
            }

            // Handle exact "startxref" line
            if (line == "startxref")
            {
                awaitingOffsetLine = true;
                continue;
            }

            // (Optional) handle "startxref 12345" (non-standard, but cheap to support)
            const std::string_view kStart = "startxref";
            const auto pos = line.find(kStart);
            if (pos != std::string_view::npos)
            {
                std::string_view rest = line.substr(pos + kStart.size());
                if (auto off = Text::ParseSizeT(rest))
                    lastXrefOffset = *off;
                else
                    awaitingOffsetLine = true;
            }
        }

        if (!lastXrefOffset.has_value())
        {
            // If you have a better error enum value (e.g., MissingStartXref), use it here.
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        // Phase 2: seek to xref offset and copy bytes until "trailer"
        _reader.Seek(*lastXrefOffset);

        std::string out;
        out.reserve(4096);

        bool firstLine = true;

        while (true)
        {
            const std::size_t read = _reader.ReadLine(buffer);
            if (read == 0)
            {
                return std::unexpected(ParserError::UnexpectedEOF);
            }

            const std::string_view rawLine{
                reinterpret_cast<const char *>(buffer.data()),
                read};

            if (firstLine)
            {
                firstLine = false;
                if (!Text::StartsWithToken(rawLine, "xref"))
                {
                    // This likely means it's an xref stream, or the file is corrupted.
                    // If you have a better error enum value (e.g., CorruptedXref), use it here.
                    return std::unexpected(ParserError::CorruptedHeader);
                }
            }
            else
            {
                if (Text::StartsWithToken(rawLine, "trailer"))
                {
                    // Stop BEFORE trailer: caller asked for xref table bytes only.
                    break;
                }
            }

            out.append(rawLine.data(), rawLine.size());
        }

        return out;
    }
}
