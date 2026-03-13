#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>

#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class content_reader
    {
    public:
        [[nodiscard]] std::expected<std::string, parser_error> read(
            reader &doc_reader,
            std::uint64_t offset) const
        {
            doc_reader.seek(offset);

            constexpr std::size_t k_buf_size = 4096;
            std::array<std::byte, k_buf_size> buffer{};
            std::string content;

            while (!doc_reader.eof())
            {
                const std::size_t bytes_read = doc_reader.read(buffer);
                if (bytes_read == 0)
                {
                    break;
                }

                content.append(reinterpret_cast<const char *>(buffer.data()), bytes_read);

                if (content.find("endobj") != std::string::npos)
                {
                    return content;
                }
            }

            return std::unexpected(parser_error::corrupted_object);
        }
    };
}
