#include "ripper/pdf/core/parser/document_structure/default_document_structure_parser.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/compressed_cross_reference_table_parser.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "ripper/pdf/core/parser/trailer/default_trailer_parser.hpp"
#include "ripper/pdf/core/util/byte.hpp"
#include "ripper/pdf/core/util/text.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>

namespace ripper::pdf::core
{
default_document_structure_parser::default_document_structure_parser(document& document)
    : default_document_structure_parser(document,
                                        std::make_unique<default_cross_reference_table_parser>(),
                                        std::make_unique<default_trailer_parser>())
{
}

default_document_structure_parser::default_document_structure_parser(
    document& document, std::unique_ptr<class cross_reference_table_parser> xref_parser,
    std::unique_ptr<class trailer_parser> trailer_parser)
    : _document{document}, _xref_parser{std::move(xref_parser)},
      _trailer_parser{std::move(trailer_parser)}
{
    if (!_xref_parser)
        _xref_parser = std::make_unique<default_cross_reference_table_parser>();
    if (!_trailer_parser)
        _trailer_parser = std::make_unique<default_trailer_parser>();
}

std::optional<std::size_t>
default_document_structure_parser::extract_prev_offset(const trailer& trailer)
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
std::optional<std::size_t>
default_document_structure_parser::find_start_xref_offset(ripper::io::core::reader& reader)
{
    constexpr std::string_view start_xref_keyword = "startxref";
    constexpr std::size_t line_buffer_size = 256;
    constexpr std::size_t search_area_size = 1024;

    const std::uint64_t file_size = reader.size();
    const std::size_t search_pos = file_size > search_area_size ? file_size - search_area_size : 0;

    reader.seek(search_pos);

    std::array<std::byte, line_buffer_size> buffer{};
    std::optional<std::uint64_t> result;

    while (!reader.eof())
    {
        const std::size_t bytes_read = reader.read_line(buffer);
        if (bytes_read == 0)
        {
            break;
        }

        const std::string_view line{byte::as_chars(buffer.data()), bytes_read};

        if (text::starts_with_token(line, start_xref_keyword))
        {
            const std::size_t offset_bytes = reader.read_line(buffer);
            if (offset_bytes == 0)
            {
                break;
            }

            const std::string_view offset_line{byte::as_chars(buffer.data()), offset_bytes};
            const auto offset = text::parse_size_t(offset_line);
            if (offset)
            {
                result = offset;
            }
            else
            {
                break;
            }
        }
    }

    return result;
}

document_structure default_document_structure_parser::parse()
{
    auto* reader_ptr = _document.reader();
    if (reader_ptr == nullptr)
        throw io_exception{"No reader backend available"};

    auto& reader = *reader_ptr;

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

        // Step 3: Detect format by reading the first few bytes
        constexpr std::size_t k_peek_size = 64;
        std::array<std::byte, k_peek_size> peek_buffer{};
        const std::size_t peek_bytes = reader.read(peek_buffer);
        if (peek_bytes == 0)
        {
            if (xref_sections.empty())
                throw parse_exception{
                    "Unable to find complete trailer while parsing document structure"};
            break;
        }

        reader.seek(current_offset);

        std::string_view peek_sv{byte::as_chars(peek_buffer.data()), peek_bytes};
        // Trim leading whitespace
        while (!peek_sv.empty() && (peek_sv.front() == ' ' || peek_sv.front() == '\n' ||
                                    peek_sv.front() == '\r' || peek_sv.front() == '\t'))
            peek_sv.remove_prefix(1);

        // Traditional xref table.
        if (text::starts_with_token(peek_sv, "xref"))
        {
            // Collect bytes until we find the end of trailer (>>)
            std::string collected_content;
            constexpr std::size_t k_buf_size = 4096;
            std::array<std::byte, k_buf_size> buffer{};
            bool has_trailer_end_been_found = false;

            while (!reader.eof() && !has_trailer_end_been_found)
            {
                const std::size_t bytes_read = reader.read(buffer);
                if (bytes_read == 0)
                    break;

                std::string_view chunk{byte::as_chars(buffer.data()), bytes_read};
                collected_content += chunk;

                if (collected_content.find("trailer") != std::string::npos &&
                    collected_content.find(">>") != std::string::npos)
                {
                    has_trailer_end_been_found = true;
                }
            }

            if (!has_trailer_end_been_found)
            {
                if (xref_sections.empty())
                    throw parse_exception{
                        "Unable to find complete trailer while parsing document structure"};
                break;
            }

            auto xref_section = _xref_parser->parse(collected_content);
            xref_section.set_startxref_offset(static_cast<std::uint64_t>(current_offset));
            xref_sections.push_back(std::move(xref_section));

            auto trailer_result = _trailer_parser->parse(collected_content);

            // Check for /XRefStm (hybrid PDF with xref stream in traditional trailer)
            const auto* xrefstm = trailer_result.dictionary().get_integer("XRefStm");
            if (xrefstm != nullptr && *xrefstm >= 0)
            {
                auto stm_offset = static_cast<std::size_t>(*xrefstm);
                if (!visited_offsets.contains(stm_offset))
                {
                    visited_offsets.insert(stm_offset);
                    reader.seek(stm_offset);

                    // Read bytes until we find "endobj"
                    std::string stm_content;
                    constexpr std::size_t k_stm_buf_size = 4096;
                    std::array<std::byte, k_stm_buf_size> stm_buffer{};
                    bool found_stm_endobj = false;

                    while (!reader.eof() && !found_stm_endobj)
                    {
                        const std::size_t stm_bytes_read = reader.read(stm_buffer);
                        if (stm_bytes_read == 0)
                            break;

                        std::string_view stm_chunk{byte::as_chars(stm_buffer.data()),
                                                   stm_bytes_read};
                        stm_content += stm_chunk;

                        if (stm_content.find("endobj") != std::string::npos)
                            found_stm_endobj = true;
                    }

                    if (found_stm_endobj)
                    {
                        auto [stm_section, stm_trailer] =
                            compressed_cross_reference_table_parser::parse(
                                _document, stm_content, indirect_reference{0, 0});
                        // Merge: stream entries take precedence
                        for (auto& entry : stm_section.entries())
                        {
                            xref_sections.back().add_entry(std::move(*entry.second));
                        }
                    }
                }
            }

            trailer_history.push_back(std::move(trailer_result));

            auto prev_offset_result = extract_prev_offset(trailer_history.back());
            if (!prev_offset_result)
                break;

            current_offset = *prev_offset_result;
        }
        // Compressed xref stream (PDF 1.5+)
        else
        {
            // Read bytes until we find "endobj"
            std::string collected_content;
            constexpr std::size_t k_buf_size = 4096;
            std::array<std::byte, k_buf_size> buffer{};
            bool found_endobj = false;

            while (!reader.eof() && !found_endobj)
            {
                const std::size_t bytes_read = reader.read(buffer);
                if (bytes_read == 0)
                    break;

                std::string_view chunk{byte::as_chars(buffer.data()), bytes_read};
                collected_content += chunk;

                if (collected_content.find("endobj") != std::string::npos)
                    found_endobj = true;
            }

            if (!found_endobj)
            {
                if (xref_sections.empty())
                    throw parse_exception{
                        "Unable to find complete trailer while parsing document structure"};
                break;
            }

            auto [section, trailer_obj] = compressed_cross_reference_table_parser::parse(
                _document, collected_content, indirect_reference{0, 0});

            section.set_startxref_offset(static_cast<std::uint64_t>(current_offset));

            xref_sections.push_back(std::move(section));
            trailer_history.push_back(std::move(trailer_obj));

            auto prev_offset_result = extract_prev_offset(trailer_history.back());
            if (!prev_offset_result)
                break;

            current_offset = *prev_offset_result;
        }
    }

    // Sections were collected newest-first (following /Prev from end of file toward beginning).
    // Reverse to chronological order (oldest first, newest last) before building the manager.
    std::reverse(xref_sections.begin(), xref_sections.end());
    std::reverse(trailer_history.begin(), trailer_history.end());

    cross_reference_manager xref_manager{std::move(xref_sections)};
    trailer_manager trailer_mgr{std::move(trailer_history)};

    return document_structure{
        std::move(xref_manager),
        std::move(trailer_mgr),
    };
}
} // namespace ripper::pdf::core
