#include "core/parser/document_structure/document_structure_parser.hpp"

#include <algorithm>
#include <array>
#include <unordered_set>

#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/util/text.hpp"

namespace ripper::core
{
    document_structure_parser::document_structure_parser(reader &reader)
        : _reader{reader}
    {
    }

    std::expected<std::size_t, parser_error> document_structure_parser::find_start_xref_offset()
    {
        constexpr std::string_view kStartXrefKeyword = "startxref";
        constexpr std::size_t kLineBufferSize = 256;
        constexpr std::size_t kSearchAreaSize = 1024;

        const std::uint64_t fileSize = _reader.size();
        const std::size_t searchPos = fileSize > kSearchAreaSize ? fileSize - kSearchAreaSize : 0;

        _reader.seek(searchPos);

        std::array<std::byte, kLineBufferSize> buffer{};
        bool foundKeyword = false;

        while (!_reader.eof())
        {
            const std::size_t bytesRead = _reader.read_line(buffer);
            if (bytesRead == 0)
            {
                break;
            }

            const std::string_view line{
                reinterpret_cast<const char *>(buffer.data()),
                bytesRead};

            if (text::starts_with_token(line, kStartXrefKeyword))
            {
                foundKeyword = true;
                break;
            }
        }

        if (!foundKeyword)
        {
            return std::unexpected(parser_error::missing_cross_reference_table);
        }

        const std::size_t bytesRead = _reader.read_line(buffer);
        if (bytesRead == 0)
        {
            return std::unexpected(parser_error::unexpected_eof);
        }

        const std::string_view offsetLine{
            reinterpret_cast<const char *>(buffer.data()),
            bytesRead};

        const auto offset = text::parse_size_t(offsetLine);
        if (!offset)
        {
            return std::unexpected(parser_error::corrupted_cross_reference_table);
        }

        return *offset;
    }

    std::expected<std::size_t, parser_error> document_structure_parser::extract_prev_offset(const trailer &trailer)
    {
        if (!trailer.prev())
        {
            return std::unexpected(parser_error::missing_cross_reference_table);
        }

        return *trailer.prev();
    }

    std::expected<document_structure_result, parser_error> document_structure_parser::parse()
    {
        // Step 1: Find startxref
        auto startXrefResult = find_start_xref_offset();
        if (!startXrefResult)
        {
            return std::unexpected(startXrefResult.error());
        }

        std::vector<cross_reference_table> xrefTableHistory;
        std::vector<trailer> trailerHistory;
        std::unordered_set<std::size_t> visitedOffsets;
        std::size_t currentOffset = *startXrefResult;

        while (true)
        {
            if (visitedOffsets.contains(currentOffset))
            {
                break;
            }
            visitedOffsets.insert(currentOffset);

            // Step 2: Collect xref and trailer bytes
            _reader.seek(currentOffset);

            // Collect bytes until we find the end of trailer (>>)
            std::string collectedContent;
            constexpr std::size_t kBufferSize = 4096;
            std::array<std::byte, kBufferSize> buffer{};
            bool foundTrailerEnd = false;

            while (!_reader.eof() && !foundTrailerEnd)
            {
                const std::size_t bytesRead = _reader.read(buffer);
                if (bytesRead == 0)
                {
                    break;
                }

                std::string_view chunk{
                    reinterpret_cast<const char *>(buffer.data()),
                    bytesRead};
                collectedContent += chunk;

                // Check if we've collected the complete trailer
                if (collectedContent.find("trailer") != std::string::npos &&
                    collectedContent.find(">>") != std::string::npos)
                {
                    foundTrailerEnd = true;
                }
            }

            if (!foundTrailerEnd)
            {
                if (xrefTableHistory.empty())
                {
                    return std::unexpected(parser_error::missing_trailer);
                }
                break;
            }

            // Step 3: parse xref from collected bytes
            default_cross_reference_table_parser xrefParser;
            auto xrefResult = xrefParser.parse(collectedContent);
            if (!xrefResult)
            {
                if (xrefTableHistory.empty())
                {
                    return std::unexpected(xrefResult.error());
                }
                break;
            }

            xrefTableHistory.push_back(std::move(xrefResult->table));

            // Step 5: parse trailer from collected bytes
            default_trailer_parser trailerParser;
            auto trailerResult = trailerParser.parse(collectedContent);
            if (!trailerResult)
            {
                if (trailerHistory.empty())
                {
                    return std::unexpected(trailerResult.error());
                }
                break;
            }

            trailerHistory.push_back(std::move(trailerResult.value()));

            // Step 6: Check for /prev to repeat
            auto prevOffsetResult = extract_prev_offset(trailerHistory.back());
            if (!prevOffsetResult)
            {
                break;
            }

            currentOffset = *prevOffsetResult;
        }

        // Compile merged xref (oldest -> newest so newer overrides)
        cross_reference_table::entry_map compiledEntries;
        for (auto it = xrefTableHistory.rbegin(); it != xrefTableHistory.rend(); ++it)
        {
            for (const auto &[objectNum, entry] : it->entries())
            {
                compiledEntries.insert_or_assign(objectNum, entry);
            }
        }

        // Compile merged trailer (oldest -> newest, set only when present)
        trailer::builder compiledTrailerBuilder{};

        for (auto it = trailerHistory.rbegin(); it != trailerHistory.rend(); ++it)
        {
            if (it->size() != 0)
            {
                compiledTrailerBuilder.size = it->size();
            }
            if (it->root().has_value())
            {
                compiledTrailerBuilder.root = it->root();
            }
            if (it->prev().has_value())
            {
                compiledTrailerBuilder.prev = it->prev();
            }
        }

        return document_structure_result{
            .compiledXrefTable = cross_reference_table{std::move(compiledEntries)},
            .xrefTableHistory = std::move(xrefTableHistory),
            .compiledTrailer = compiledTrailerBuilder.build(),
            .trailerHistory = std::move(trailerHistory),
        };
    }
}
