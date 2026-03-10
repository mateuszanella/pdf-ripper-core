#include "core/parser/header/header_parser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string_view>

#include "core/reader/reader.hpp"

namespace ripper::core
{
    header_parser::header_parser(reader &reader)
        : _reader{reader}
    {
    }

    std::expected<header, parser_error> header_parser::parse()
    {
        constexpr std::string_view kMagic = "%PDF-";
        constexpr std::size_t kMaxHeaderLineLength = 256;

        std::array<std::byte, kMaxHeaderLineLength> buffer{};

        _reader.seek(0);

        const std::size_t headerStartPos = _reader.tell();

        const std::size_t read = _reader.read_line(buffer);
        if (read == 0)
        {
            return std::unexpected(parser_error::unexpected_eof);
        }

        const std::string_view line{
            reinterpret_cast<const char *>(buffer.data()),
            read};

        const std::size_t pos = line.find(kMagic);
        if (pos == std::string_view::npos)
        {
            return std::unexpected(parser_error::missing_header);
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
            return std::unexpected(parser_error::corrupted_header);
        }

        const std::size_t headerEndPos = headerStartPos + pos + kMagic.size() + len;

        return header{std::string{rest.substr(0, len)}};
    }
}
