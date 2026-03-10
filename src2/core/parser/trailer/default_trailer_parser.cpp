#include "core/parser/trailer/default_trailer_parser.hpp"

#include <string_view>

#include "core/util/text.hpp"

namespace ripper::core
{
    std::expected<std::pair<std::uint32_t, std::uint16_t>, parser_error>
        default_trailer_parser::parse_indirect_reference(std::string_view line)
    {
        // parse "objNum genNum R"
        const std::size_t firstSpace = line.find(' ');
        if (firstSpace == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_trailer);
        }

        const auto objNum = text::parse_size_t(line.substr(0, firstSpace));
        if (!objNum)
        {
            return std::unexpected(parser_error::corrupted_trailer);
        }

        line = line.substr(firstSpace + 1);
        line = text::trim_ascii(line);

        const std::size_t secondSpace = line.find(' ');
        if (secondSpace == std::string_view::npos)
        {
            return std::unexpected(parser_error::corrupted_trailer);
        }

        const auto genNum = text::parse_size_t(line.substr(0, secondSpace));
        if (!genNum)
        {
            return std::unexpected(parser_error::corrupted_trailer);
        }

        return std::make_pair(
            static_cast<std::uint32_t>(*objNum),
            static_cast<std::uint16_t>(*genNum)
        );
    }

    std::expected<trailer, parser_error> default_trailer_parser::parse_dictionary(
        std::string_view content)
    {
        trailer trailer;

        // parse /size
        if (const std::size_t sizePos = content.find("/Size"); sizePos != std::string_view::npos)
        {
            std::string_view rest = content.substr(sizePos + 5);
            rest = text::trim_ascii(rest);

            const auto size = text::parse_size_t(rest);
            if (size)
            {
                trailer.set_size(static_cast<std::uint32_t>(*size));
            }
        }

        // parse /prev
        if (const std::size_t prevPos = content.find("/Prev"); prevPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(prevPos + 5);
            rest = text::trim_ascii(rest);

            const auto prev = text::parse_size_t(rest);
            if (prev)
            {
                trailer.set_prev(*prev);
            }
        }

        // parse /Root
        if (const std::size_t rootPos = content.find("/Root"); rootPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(rootPos + 5);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                trailer.set_root(ref->first, ref->second);
            }
        }

        // parse /Info
        if (const std::size_t infoPos = content.find("/Info"); infoPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(infoPos + 5);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                trailer.set_info(ref->first, ref->second);
            }
        }

        // parse /Encrypt
        if (const std::size_t encryptPos = content.find("/Encrypt"); encryptPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(encryptPos + 8);
            rest = text::trim_ascii(rest);

            auto ref = parse_indirect_reference(rest);
            if (ref)
            {
                trailer.set_encrypt(ref->first, ref->second);
            }
        }

        // parse /id
        if (const std::size_t idPos = content.find("/ID"); idPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(idPos + 3);
            rest = text::trim_ascii(rest);

            // Look for array opening bracket
            if (const std::size_t arrayStart = rest.find('['); arrayStart != std::string_view::npos)
            {
                rest = rest.substr(arrayStart + 1);

                std::string firstId;
                std::optional<std::string> secondId;

                // parse first hex string (required)
                rest = text::trim_ascii(rest);
                if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
                {
                    if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                    {
                        firstId = std::string{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                        rest = rest.substr(hexEnd + 1);
                    }
                }

                // parse second hex string (optional)
                rest = text::trim_ascii(rest);
                if (const std::size_t hexStart = rest.find('<'); hexStart != std::string_view::npos)
                {
                    if (const std::size_t hexEnd = rest.find('>', hexStart + 1); hexEnd != std::string_view::npos)
                    {
                        secondId = std::string{rest.substr(hexStart + 1, hexEnd - hexStart - 1)};
                    }
                }

                if (!firstId.empty())
                {
                    trailer.set_id({firstId, secondId});
                }
            }
        }

        return trailer;
    }

    std::expected<trailer, parser_error> default_trailer_parser::parse(std::string_view content)
    {
        // Find "trailer" keyword
        const std::size_t trailerPos = content.find("trailer");
        if (trailerPos == std::string_view::npos)
        {
            return std::unexpected(parser_error::missing_trailer);
        }

        content = content.substr(trailerPos + 7); // skip "trailer"

        // Find dictionary delimiters
        const std::size_t startPos = content.find("<<");
        const std::size_t endPos = content.find(">>");

        if (startPos == std::string_view::npos || endPos == std::string_view::npos || endPos <= startPos)
        {
            return std::unexpected(parser_error::corrupted_trailer);
        }

        std::string_view dictContent = content.substr(startPos + 2, endPos - startPos - 2);

        return parse_dictionary(dictContent);
    }
}
