#include "Core/Parser/Parser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string_view>
#include <stdexcept>
#include <mutex>
#include <unordered_map>

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

    const Parser::Breakpoints& Parser::GetBreakpoints() const
    {
        return _breakpoints;
    }

    /**
     * @todo should handle multiple xrefs, not this method, just this in general
     * @todo this looks shit
     */
    std::expected<std::size_t, ParserError> Parser::FindLastStartXrefOffset()
    {
        constexpr std::size_t kMaxLineLength = 8192;
        std::array<std::byte, kMaxLineLength> buffer{};

        _reader.Seek(0);

        std::optional<std::size_t> lastXrefOffset;
        bool awaitingOffsetLine = false;

        std::size_t curPos = 0;
        std::size_t maybeStartKeywordPos = 0;

        while (true)
        {
            const std::size_t read = _reader.ReadLine(buffer);
            if (read == 0)
                break;

            const std::string_view rawLine{
                reinterpret_cast<const char *>(buffer.data()),
                read};

            const std::string_view line = Text::TrimAscii(Text::StripLineEndings(rawLine));

            if (awaitingOffsetLine)
            {
                _breakpoints.lastStartXrefOffsetLinePos = curPos;
                if (auto off = Text::ParseSizeT(line))
                {
                    lastXrefOffset = off.value();
                    _breakpoints.lastStartXrefKeywordPos = maybeStartKeywordPos;
                }
                awaitingOffsetLine = false;
                curPos += read;
                continue;
            }

            if (line == "startxref")
            {
                maybeStartKeywordPos = curPos;
                awaitingOffsetLine = true;
                curPos += read;
                continue;
            }

            constexpr std::string_view kStart = "startxref";
            const auto pos = line.find(kStart);
            if (pos != std::string_view::npos)
            {
                maybeStartKeywordPos = curPos;

                std::string_view rest = line.substr(pos + kStart.size());
                if (auto off = Text::ParseSizeT(rest))
                {
                    lastXrefOffset = off.value();
                    _breakpoints.lastStartXrefKeywordPos = curPos;
                    _breakpoints.lastStartXrefOffsetLinePos = curPos;
                }
                else
                {
                    awaitingOffsetLine = true;
                }
            }

            curPos += read;
        }

        _breakpoints.eofPos = curPos;

        if (!lastXrefOffset.has_value())
        {
            return std::unexpected(ParserError::UnexpectedEOF);
        }

        _breakpoints.xrefOffset = *lastXrefOffset;
        return *lastXrefOffset;
    }

    std::expected<std::string, ParserError> Parser::ReadXrefTableAtOffset(std::size_t xrefOffset)
    {
        constexpr std::size_t kMaxLineLength = 8192;
        std::array<std::byte, kMaxLineLength> buffer{};

        _reader.Seek(xrefOffset);

        std::string out;
        out.reserve(4096);

        bool firstLine = true;
        std::size_t curPos = xrefOffset;

        _breakpoints.xrefStartPos = xrefOffset;

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
                    return std::unexpected(ParserError::CorruptedHeader);
                }
            }
            else
            {
                if (Text::StartsWithToken(rawLine, "trailer"))
                {
                    _breakpoints.xrefEndPos = curPos;
                    _breakpoints.trailerKeywordPos = curPos;
                    break;
                }
            }

            out.append(rawLine.data(), rawLine.size());
            curPos += read;
        }

        return out;
    }

    std::expected<std::string, ParserError> Parser::ReadCrossReferenceTable()
    {
        auto offExp = FindLastStartXrefOffset();
        if (!offExp)
        {
            return std::unexpected(offExp.error());
        }

        auto tableExp = ReadXrefTableAtOffset(offExp.value());
        if (!tableExp)
        {
            return std::unexpected(tableExp.error());
        }

        return *tableExp;
    }
}
