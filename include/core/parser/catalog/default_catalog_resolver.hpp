#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/catalog_parser.hpp"
#include "core/parser/catalog/default_catalog_parser.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class default_catalog_resolver
    {
    public:
        [[nodiscard]] std::expected<catalog_parse_result, parser_error> parse(
            reader &reader,
            const cross_reference_table &xref_table,
            const trailer &trailer_obj) const
        {
            if (!trailer_obj.root().has_value())
                return std::unexpected(parser_error::missing_catalog);

            const auto &catalog_ref = *trailer_obj.root();

            auto offset_result = lookup_offset(xref_table, catalog_ref);
            if (!offset_result)
                return std::unexpected(offset_result.error());

            auto content_result = read_object_content(reader, *offset_result);
            if (!content_result)
                return std::unexpected(content_result.error());

            default_catalog_parser object_parser{};
            return object_parser.parse(*content_result, catalog_ref);
        }

    private:
        [[nodiscard]] std::expected<std::string, parser_error> read_object_content(
            reader &reader,
            std::uint64_t offset) const
        {
            reader.seek(offset);

            constexpr std::size_t kBufferSize = 4096;
            std::array<std::byte, kBufferSize> buffer{};
            std::string content;

            bool found_endobj = false;
            while (!reader.eof() && !found_endobj)
            {
                const std::size_t bytes_read = reader.read(buffer);
                if (bytes_read == 0)
                    break;

                std::string_view chunk{reinterpret_cast<const char *>(buffer.data()), bytes_read};
                content += chunk;

                if (content.find("endobj") != std::string::npos)
                    found_endobj = true;
            }

            if (!found_endobj)
                return std::unexpected(parser_error::corrupted_object);

            return content;
        }

        [[nodiscard]] std::expected<std::uint64_t, parser_error> lookup_offset(
            const cross_reference_table &xref_table,
            const indirect_reference &ref) const
        {
            auto entry = xref_table.find(ref);
            if (!entry.has_value() || !entry->in_use())
                return std::unexpected(parser_error::object_not_found);

            return entry->offset();
        }
    };
}
