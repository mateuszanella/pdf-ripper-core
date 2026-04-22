#include "core/parser/document_structure/default_document_structure_parser.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    default_document_structure_parser::default_document_structure_parser(reader &reader)
        : default_document_structure_parser(
              reader,
              std::make_unique<default_cross_reference_table_parser>(),
              std::make_unique<default_trailer_parser>())
    {
    }

    default_document_structure_parser::default_document_structure_parser(
        reader &reader,
        std::unique_ptr<class cross_reference_table_parser> xref_parser,
        std::unique_ptr<class trailer_parser> trailer_parser)
        : _reader{reader},
          _xref_parser{std::move(xref_parser)},
          _trailer_parser{std::move(trailer_parser)}
    {
        if (!_xref_parser)
            _xref_parser = std::make_unique<default_cross_reference_table_parser>();
        if (!_trailer_parser)
            _trailer_parser = std::make_unique<default_trailer_parser>();
    }

    /**
     * @todo Technically, this implementation does not really get the last startxref,
     *       since the startxref keyword could appear in the last 1024 bytes multiple times.
     */
    std::expected<std::size_t, error> default_document_structure_parser::find_start_xref_offset()
    {
        constexpr std::string_view start_xref_keyword = "startxref";
        constexpr std::size_t line_buffer_size = 256;
        constexpr std::size_t search_area_size = 1024;

        const std::uint64_t file_size = _reader.size();
        const std::size_t search_pos = file_size > search_area_size
                                           ? file_size - search_area_size
                                           : 0;

        _reader.seek(search_pos);

        std::array<std::byte, line_buffer_size> buffer{};
        bool found_keyword = false;

        while (!_reader.eof())
        {
            const std::size_t bytes_read = _reader.read_line(buffer);
            if (bytes_read == 0)
            {
                break;
            }

            const std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytes_read};

            if (text::starts_with_token(line, start_xref_keyword))
            {
                found_keyword = true;
                break;
            }
        }

        if (!found_keyword)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_xref_table)
                                       .with_component(error_component::cross_reference)
                                       .with_field("startxref")
                                       .with_expected("startxref section")
                                       .with_message("Missing startxref section")
                                       .build());
        }

        const std::size_t bytes_read = _reader.read_line(buffer);
        if (bytes_read == 0)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::unexpected_eof)
                                       .with_component(error_component::cross_reference)
                                       .with_field("startxref_offset")
                                       .with_expected("numeric offset line")
                                       .with_message("Missing startxref offset line")
                                       .build());
        }

        const std::string_view offset_line{
            reinterpret_cast<const char *>(buffer.data()),
            bytes_read};

        const auto offset = text::parse_size_t(offset_line);
        if (!offset)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_xref_table)
                                       .with_component(error_component::cross_reference)
                                       .with_field("startxref_offset")
                                       .with_actual(std::string{offset_line})
                                       .with_message("Invalid startxref offset")
                                       .build());
        }

        return offset.value();
    }

    std::expected<std::size_t, error> default_document_structure_parser::extract_prev_offset(const trailer &trailer)
    {
        if (!trailer.prev())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_xref_table)
                                       .with_component(error_component::trailer)
                                       .with_field("Prev")
                                       .with_expected("offset")
                                       .with_actual("missing")
                                       .with_message("Trailer has no Prev offset")
                                       .build());
        }

        return trailer.prev().value();
    }

    std::expected<document_structure_result, error> default_document_structure_parser::parse()
    {
        // Step 1: Find the last occurence of startxref from the end of the file
        auto start_xref_result = find_start_xref_offset();
        if (!start_xref_result)
        {
            return std::unexpected(start_xref_result.error());
        }

        std::vector<cross_reference_table> xref_history;
        std::vector<trailer> trailer_history;
        std::unordered_set<std::size_t> visited_offsets;
        std::size_t current_offset = start_xref_result.value();

        // In this main loop, we iterate through the whole chain of xref/trailer
        // pairs starting from the last one (pointed by startxref) and following
        // /Prev links until we reach the end of the chain or encounter an error.
        for (;;)
        {
            // Ensure we don't loop infinitely in case of circular /Prev references
            if (visited_offsets.contains(current_offset))
            {
                break;
            }

            visited_offsets.insert(current_offset);

            // Step 2: Seek to the startxref offset of the current xref/trailer pair
            _reader.seek(current_offset);

            // Collect bytes until we find the end of trailer (>>)
            std::string collected_content;
            constexpr std::size_t k_buf_size = 4096;
            std::array<std::byte, k_buf_size> buffer{};
            bool has_trailer_end_been_found = false;

            while (!_reader.eof() && !has_trailer_end_been_found)
            {
                const std::size_t bytes_read = _reader.read(buffer);
                if (bytes_read == 0)
                {
                    break;
                }

                std::string_view chunk{
                    reinterpret_cast<const char *>(buffer.data()),
                    bytes_read};
                collected_content += chunk;

                // Check if we've collected the complete trailer
                if (collected_content.find("trailer") != std::string::npos &&
                    collected_content.find(">>") != std::string::npos)
                {
                    has_trailer_end_been_found = true;
                }
            }

            // If we couldn't find a complete trailer, we can't continue parsing this pair.
            // If it's the first pair, we consider this a fatal error, otherwise we just stop
            // and return what we have so far.
            if (!has_trailer_end_been_found)
            {
                if (xref_history.empty())
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::missing_trailer)
                                               .with_component(error_component::document)
                                               .with_field("trailer")
                                               .with_expected("complete trailer dictionary")
                                               .with_message("Unable to find complete trailer while parsing document structure")
                                               .build());
                }
                break;
            }

            // Step 3: Parse xref from collected bytes
            auto cross_reference_table = _xref_parser->parse(collected_content);
            if (!cross_reference_table)
            {
                if (xref_history.empty())
                {
                    return std::unexpected(cross_reference_table.error());
                }
                break;
            }

            xref_history.push_back(std::move(cross_reference_table.value()));

            // Step 4: Parse trailer from collected bytes
            auto trailerResult = _trailer_parser->parse(collected_content);
            if (!trailerResult)
            {
                if (trailer_history.empty())
                {
                    return std::unexpected(trailerResult.error());
                }
                break;
            }

            trailer_history.push_back(std::move(trailerResult.value()));

            // Step 5: Check for /Prev to repeat
            auto prev_offset_result = extract_prev_offset(trailer_history.back());
            if (!prev_offset_result)
            {
                break;
            }

            current_offset = *prev_offset_result;
        }

        // Compile merged xref (oldest -> newest so newer overrides)
        cross_reference_table::entry_map compiled_entries;
        for (auto it = xref_history.rbegin(); it != xref_history.rend(); ++it)
        {
            for (const auto &[objectNum, entry] : it->entries())
            {
                compiled_entries.insert_or_assign(objectNum, entry);
            }
        }

        // Compile merged trailer (oldest -> newest, set only when present)
        trailer::builder compiled_trailer_builder{};

        for (auto it = trailer_history.rbegin(); it != trailer_history.rend(); ++it)
        {
            if (it->size() != 0)
            {
                compiled_trailer_builder.size = it->size();
            }
            if (it->root().has_value())
            {
                compiled_trailer_builder.root = it->root();
            }
            if (it->prev().has_value())
            {
                compiled_trailer_builder.prev = it->prev();
            }
            if (it->id().has_value())
            {
                compiled_trailer_builder.id = it->id();
            }
        }

        return document_structure_result{
            .compiled_xref = cross_reference_table{std::move(compiled_entries)},
            .xref_history = std::move(xref_history),
            .compiled_trailer = compiled_trailer_builder.build(),
            .trailer_history = std::move(trailer_history),
        };
    }
}
