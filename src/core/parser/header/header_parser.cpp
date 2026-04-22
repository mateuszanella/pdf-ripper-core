#include "core/parser/header/header_parser.hpp"

#include <array>
#include <cctype>
#include <expected>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    header_parser::header_parser(reader &reader)
        : _reader{reader}
    {
    }

    std::expected<header, error> header_parser::parse()
    {
        constexpr std::string_view kMagic = "%PDF-";
        constexpr std::size_t kMaxHeaderLineLength = 256;

        std::array<std::byte, kMaxHeaderLineLength> buffer{};

        _reader.seek(0);

        const std::size_t read = _reader.read_line(buffer);
        if (read == 0)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unexpected_eof)
                                       .with_component(error_component::parser)
                                       .with_offset(0)
                                       .with_field("header")
                                       .with_expected("%PDF-")
                                       .with_actual("empty")
                                       .with_message("File is empty while reading PDF header")
                                       .build());
        }

        const std::string_view line{
            reinterpret_cast<const char *>(buffer.data()),
            read};

        const std::size_t pos = line.find(kMagic);
        if (pos == std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_header)
                                       .with_component(error_component::parser)
                                       .with_offset(0)
                                       .with_field("header_signature")
                                       .with_expected("%PDF-")
                                       .with_actual(std::string{line})
                                       .with_message("Missing PDF header signature")
                                       .build());
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
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_header)
                                       .with_component(error_component::parser)
                                       .with_offset(pos + kMagic.size())
                                       .with_field("header_version")
                                       .with_expected("digit[.digit]")
                                       .with_actual(std::string{rest})
                                       .with_message("Invalid PDF header version")
                                       .build());
        }

        const std::string_view version = rest.substr(0, len);
        const std::size_t dotPos = version.find('.');
        if (dotPos == std::string_view::npos || dotPos == 0 || dotPos + 1 >= version.size())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_header)
                                       .with_component(error_component::parser)
                                       .with_offset(pos + kMagic.size())
                                       .with_field("header_version")
                                       .with_expected("major.minor")
                                       .with_actual(std::string{version})
                                       .with_message("Invalid PDF header version format")
                                       .build());
        }

        if (version.find('.', dotPos + 1) != std::string_view::npos)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_header)
                                       .with_component(error_component::parser)
                                       .with_offset(pos + kMagic.size())
                                       .with_field("header_version")
                                       .with_expected("single dot in major.minor")
                                       .with_actual(std::string{version})
                                       .with_message("Invalid PDF header version format")
                                       .build());
        }

        return header{std::string{version}};
    }
}
