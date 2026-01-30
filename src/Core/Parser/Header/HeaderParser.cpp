#include "Core/Parser/Header/HeaderParser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string_view>

#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    HeaderParser::HeaderParser(Reader &reader)
        : _reader{reader}
    {
    }

    std::expected<Header, ParserError> HeaderParser::Parse()
    {
        constexpr std::string_view kMagic = "%PDF-";
        constexpr std::size_t kMaxHeaderLineLength = 256;

        std::array<std::byte, kMaxHeaderLineLength> buffer{};

        _reader.Seek(0);

        const std::size_t headerStartPos = _reader.Tell();

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

        const std::size_t headerEndPos = headerStartPos + pos + kMagic.size() + len;

        return Header{std::string{rest.substr(0, len)}};
    }
}
