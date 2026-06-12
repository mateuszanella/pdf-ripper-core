#include "core/parser/indirect_object_resolver.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

#include "core/document.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/util/text.hpp"

namespace ripper::pdf::core
{
    namespace
    {
        struct observed_token
        {
            lexer_token token{};
            std::size_t offset{std::string::npos};
        };

        std::size_t token_offset_in(std::string_view source, const lexer_token &token)
        {
            if (token.lexeme.empty())
            {
                return std::string::npos;
            }

            const char *begin = source.data();
            const char *end = source.data() + source.size();
            const char *ptr = token.lexeme.data();

            if (ptr < begin || ptr >= end)
            {
                return std::string::npos;
            }

            return static_cast<std::size_t>(ptr - begin);
        }
    }

    indirect_object_resolver::indirect_object_resolver(document &document)
        : document_{document}
    {
    }

    std::string indirect_object_resolver::resolve(indirect_reference ref) const
    {
        auto *reader_ptr = document_.reader();
        if (!reader_ptr)
            throw io_exception{"No reader backend available"};

        auto &r = *reader_ptr;

        if (!r.is_open())
            throw io_exception{"The provided reader is not open while trying to resolve an indirect object"};

        auto &xref = document_.cross_reference_table();
        const auto entry = xref.find(ref);
        if (!entry)
            throw parse_exception{"XRef entry not found for indirect object"};

        if (!entry->in_use())
            throw parse_exception{"XRef entry is not in use"};

        if (entry->reference().generation() != ref.generation())
            throw parse_exception{"Generation mismatch for indirect object"};

        const std::uint64_t file_size_u64 = r.size();
        if (file_size_u64 == 0)
            throw io_exception{"Read size zero while trying to resolve an indirect object"};

        const auto offset_u64 = entry->offset();
        if (!offset_u64.has_value())
            throw parse_exception{"XRef entry is missing file offset"};

        if (offset_u64.value() >= file_size_u64)
            throw parse_exception{"Object offset is out of bounds"};

        if (offset_u64.value() > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()))
            throw parse_exception{"Object offset is too large to handle"};

        const std::size_t offset = static_cast<std::size_t>(offset_u64.value());
        const std::size_t to_read = static_cast<std::size_t>(file_size_u64 - offset_u64.value());

        std::vector<std::byte> bytes(to_read);
        const std::size_t read = r.read_at(std::span<std::byte>{bytes.data(), bytes.size()}, offset);
        if (read == 0)
            throw io_exception{"Received zero bytes from reader while attempting to read indirect object content"};

        std::string source(read, '\0');
        for (std::size_t i = 0; i < read; ++i)
        {
            source[i] = static_cast<char>(bytes[i]);
        }

        pdf_lexer lexer{source};

        std::array<observed_token, 3> window{};
        std::size_t window_size = 0;

        auto push_token = [&](const observed_token &t)
        {
            if (window_size < window.size())
            {
                window[window_size++] = t;
                return;
            }

            window[0] = window[1];
            window[1] = window[2];
            window[2] = t;
        };

        while (true)
        {
            const auto token = lexer.next();
            if (token.type == lexer_token_type::eof)
            {
                break;
            }

            observed_token current{
                .token = token,
                .offset = token_offset_in(source, token),
            };
            push_token(current);

            if (window_size < 3)
            {
                continue;
            }

            const auto &a = window[0];
            const auto &b = window[1];
            const auto &c = window[2];

            if (a.token.type != lexer_token_type::integer ||
                b.token.type != lexer_token_type::integer ||
                c.token.type != lexer_token_type::keyword ||
                c.token.lexeme != "obj" ||
                a.offset == std::string::npos)
            {
                continue;
            }

            const auto object_number = text::parse_u32(a.token.lexeme);
            const auto generation = text::parse_u16(b.token.lexeme);

            if (!object_number || !generation)
            {
                continue;
            }

            if (*object_number != ref.object_number() || *generation != ref.generation())
            {
                continue;
            }

            const std::size_t object_start = a.offset;

            while (true)
            {
                const auto end_token = lexer.next();
                if (end_token.type == lexer_token_type::eof)
                    throw parse_exception{"Unexpected end of file while scanning to endobj"};

                if (end_token.type == lexer_token_type::keyword && end_token.lexeme == "endobj")
                {
                    const std::size_t end_offset = token_offset_in(source, end_token);
                    if (end_offset == std::string::npos || end_offset < object_start)
                        throw parse_exception{"Invalid indirect object end offset"};

                    const std::size_t object_end = end_offset + end_token.lexeme.size();
                    return source.substr(object_start, object_end - object_start);
                }
            }
        }

        throw parse_exception{"Indirect object not found"};
    }
}
