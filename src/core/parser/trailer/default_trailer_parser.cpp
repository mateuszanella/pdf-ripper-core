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
        trailer::builder trailer_builder{};

        // parse /Size
        if (const std::size_t sizePos = content.find("/Size"); sizePos != std::string_view::npos)
        {
            std::string_view rest = content.substr(sizePos + 5);
            rest = text::trim_ascii(rest);

            const auto size = text::parse_size_t(rest);
            if (size)
            {
                trailer_builder.size = static_cast<std::uint64_t>(*size);
            }
        }

        // parse /Prev
        if (const std::size_t prevPos = content.find("/Prev"); prevPos != std::string_view::npos)
        {
            std::string_view rest = content.substr(prevPos + 5);
            rest = text::trim_ascii(rest);

            const auto prev = text::parse_size_t(rest);
            if (prev)
            {
                trailer_builder.prev = static_cast<std::uint64_t>(*prev);
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
                trailer_builder.root = indirect_reference{ref->first, ref->second};
            }
        }

        // /Info, /Encrypt and /ID parsing intentionally deferred (not in trailer model yet).
        return trailer_builder.build();
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
