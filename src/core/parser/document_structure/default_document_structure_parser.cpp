#include "core/parser/document_structure/default_document_structure_parser.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/cross_reference_table/cross_reference_section.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/util/text.hpp"
#include "core/document.hpp"

namespace ripper::pdf::core
{
    default_document_structure_parser::default_document_structure_parser(document &document)
        : default_document_structure_parser(
              document,
              std::make_unique<default_cross_reference_table_parser>(),
              std::make_unique<default_trailer_parser>())
    {
    }

    default_document_structure_parser::default_document_structure_parser(
        document &document,
        std::unique_ptr<class cross_reference_table_parser> xref_parser,
        std::unique_ptr<class trailer_parser> trailer_parser)
        : _document{document},
          _xref_parser{std::move(xref_parser)},
          _trailer_parser{std::move(trailer_parser)}
    {
        if (!_xref_parser)
            _xref_parser = std::make_unique<default_cross_reference_table_parser>();
        if (!_trailer_parser)
            _trailer_parser = std::make_unique<default_trailer_parser>();
    }

    std::optional<std::size_t> default_document_structure_parser::extract_prev_offset(const trailer &trailer)
    {
        auto prev = trailer.prev();
        if (!prev)
            return std::nullopt;

        return static_cast<std::size_t>(*prev);
    }

    /**
     * @todo Technically, this implementation does not really get the last startxref,
     *       since the startxref keyword could appear in the last 1024 bytes multiple times.
     */
    std::optional<std::size_t> default_document_structure_parser::find_start_xref_offset(reader &reader)
    {
        constexpr std::string_view start_xref_keyword = "startxref";
        constexpr std::size_t line_buffer_size = 256;
        constexpr std::size_t search_area_size = 1024;

        const std::uint64_t file_size = reader.size();
        const std::size_t search_pos = file_size > search_area_size
                                           ? file_size - search_area_size
                                           : 0;

        reader.seek(search_pos);

        std::array<std::byte, line_buffer_size> buffer{};
        bool found_keyword = false;

        while (!reader.eof())
        {
            const std::size_t bytes_read = reader.read_line(buffer);
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
            return std::nullopt;
        }

        const std::size_t bytes_read = reader.read_line(buffer);
        if (bytes_read == 0)
        {
            throw parse_exception{"Missing startxref offset line"};
        }

        const std::string_view offset_line{
            reinterpret_cast<const char *>(buffer.data()),
            bytes_read};

        const auto offset = text::parse_size_t(offset_line);
        if (!offset)
        {
            throw parse_exception{"Invalid startxref offset"};
        }

        return offset.value();
    }

    document_structure default_document_structure_parser::parse()
    {
        auto *reader_ptr = _document.reader();
        if (!reader_ptr)
            throw io_exception{"No reader backend available"};

        auto &reader = *reader_ptr;

        auto start_xref_result = find_start_xref_offset(reader);
        if (!start_xref_result)
            throw parse_exception{"Missing startxref section"};

        std::vector<cross_reference_section> xref_sections;
        std::vector<trailer> trailer_history;
        std::unordered_set<std::size_t> visited_offsets;
        std::size_t current_offset = *start_xref_result;

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
            reader.seek(current_offset);

            // Collect bytes until we find the end of trailer (>>)
            std::string collected_content;
            constexpr std::size_t k_buf_size = 4096;
            std::array<std::byte, k_buf_size> buffer{};
            bool has_trailer_end_been_found = false;

            while (!reader.eof() && !has_trailer_end_been_found)
            {
                const std::size_t bytes_read = reader.read(buffer);
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
                if (xref_sections.empty())
                {
                    throw parse_exception{"Unable to find complete trailer while parsing document structure"};
                }
                break;
            }

            // Step 3: Parse xref section from collected bytes
            auto xref_section = _xref_parser->parse(collected_content);
            xref_section.set_startxref_offset(static_cast<std::uint64_t>(current_offset));
            xref_sections.push_back(std::move(xref_section));

            // Step 4: Parse trailer from collected bytes
            auto trailerResult = _trailer_parser->parse(collected_content);
            trailer_history.push_back(std::move(trailerResult));

            // Step 5: Check for /Prev to repeat
            auto prev_offset_result = extract_prev_offset(trailer_history.back());
            if (!prev_offset_result)
            {
                break;
            }

            current_offset = *prev_offset_result;
        }

        // Sections were collected newest-first (following /Prev from end of file toward beginning).
        // Reverse to chronological order (oldest first, newest last) before building the manager.
        std::reverse(xref_sections.begin(), xref_sections.end());

        cross_reference_manager xref_manager{std::move(xref_sections)};

        // Compile merged trailer: trailer_history[0] = newest, back = oldest.
        // Iterate oldest-to-newest (rbegin → rend) so that newer values overwrite older ones.
        dictionary compiled_dict{};

        for (auto it = trailer_history.rbegin(); it != trailer_history.rend(); ++it)
        {
            for (const auto &[key, val] : it->dictionary().entries())
            {
                compiled_dict.set(key, val);
            }
        }

        return document_structure{
            std::move(xref_manager),
            trailer{std::move(compiled_dict)},
            std::move(trailer_history),
        };
    }
}
