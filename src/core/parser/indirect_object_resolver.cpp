#include "core/parser/indirect_object_resolver.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <vector>

#include "core/document.hpp"
#include "core/error.hpp"
#include "core/error_builder.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/reader/reader.hpp"
#include "core/util/text.hpp"

namespace ripper::core
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

    indirect_object_resolver::indirect_object_resolver(const document &document) noexcept
        : document_{document}
    {
    }

    std::expected<std::string, error> indirect_object_resolver::resolve(indirect_reference ref) const
    {
        auto &r = document_.reader();
        if (!r.is_open())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::reader_not_open)
                                       .with_component(error_component::reader)
                                       .with_message("The provided reader is not open while trying to resolve an indirect object")
                                       .build());
        }

        // Resolve the object location from the compiled xref table. We only proceed
        // with an in-use entry and the exact generation requested by the caller, so
        // we do not accidentally return another revision.
        const auto xref = document_.cross_reference_table();
        if (!xref)
        {
            return std::unexpected(xref.error());
        }

        const auto entry = xref->find(ref);
        if (!entry.has_value())
        {
            return std::unexpected(error_builder::xref_entry_not_found(ref));
        }

        if (!entry->in_use())
        {
            return std::unexpected(error_builder::xref_entry_not_in_use(ref));
        }

        if (entry->reference().generation() != ref.generation())
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::generation_mismatch)
                                       .with_component(error_component::cross_reference)
                                       .with_reference(ref)
                                       .with_message("Generation mismatch for indirect object")
                                       .build());
        }

        const std::uint64_t file_size_u64 = r.size();
        if (file_size_u64 == 0)
        {
            const std::string error_message = "Read size zero when trying to resolve indirect object " +
                                              std::to_string(ref.object_number()) + " " +
                                              std::to_string(ref.generation());

            return std::unexpected(error_builder::create()
                                       .with_code(error_code::io_error)
                                       .with_component(error_component::reader)
                                       .with_reference(ref)
                                       .with_message(error_message)
                                       .build());
        }

        const std::uint64_t offset_u64 = entry->offset();
        if (offset_u64 >= file_size_u64)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::offset_out_of_bounds)
                                       .with_component(error_component::cross_reference)
                                       .with_reference(ref)
                                       .with_message("Offset is out of bounds")
                                       .build());
        }

        if (offset_u64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()))
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::offset_out_of_bounds)
                                       .with_component(error_component::cross_reference)
                                       .with_reference(ref)
                                       .with_message("Offset is too large to handle")
                                       .build());
        }

        // Read from the xref-resolved byte offset to EOF. This narrows parsing
        // to the region where the target indirect object is expected to exist,
        // instead of scanning from byte 0.
        const std::size_t offset = static_cast<std::size_t>(offset_u64);
        const std::size_t to_read = static_cast<std::size_t>(file_size_u64 - offset_u64);

        std::vector<std::byte> bytes(to_read);
        const std::size_t read = r.read_at(std::span<std::byte>{bytes.data(), bytes.size()}, offset);
        if (read == 0)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::io_error)
                                       .with_component(error_component::reader)
                                       .with_reference(ref)
                                       .with_offset(offset)
                                       .with_message("Received zero bytes from reader while attempting to read at offset: " + std::to_string(offset))
                                       .build());
        }

        std::string source(read, '\0');
        for (std::size_t i = 0; i < read; ++i)
        {
            source[i] = static_cast<char>(bytes[i]);
        }

        pdf_lexer lexer{source};

        // We then scan the source for an indirect object header:
        //
        //   "<object_number> <generation> obj"
        //
        // We keep a 3-token sliding window and compare parsed numbers to
        // `ref` to ensure that we parse the exact object revision requested
        //  by the caller.
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
            auto token_result = lexer.next();
            if (!token_result)
            {
                return std::unexpected(error_builder::create()
                                           .with_code(error_code::tokenization_error)
                                           .with_component(error_component::lexer)
                                           .with_reference(ref)
                                           .with_offset(offset)
                                           .with_message("Failed to parse token")
                                           .build());
            }

            const auto token = *token_result;
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

            // Header match gate: confirms this is exactly "N G obj" for `ref`.
            if (*object_number != ref.object_number() || *generation != ref.generation())
            {
                continue;
            }

            const std::size_t object_start = a.offset;

            // After matching the header, continue lexing until "endobj" and return
            // the exact source slice covering the full indirect object.
            while (true)
            {
                auto end_token_result = lexer.next();
                if (!end_token_result)
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::tokenization_error)
                                               .with_component(error_component::lexer)
                                               .with_reference(ref)
                                               .with_offset(offset + object_start)
                                               .with_message("Failed to parse token while scanning to endobj")
                                               .build());
                }

                const auto end_token = *end_token_result;
                if (end_token.type == lexer_token_type::eof)
                {
                    return std::unexpected(error_builder::create()
                                               .with_code(error_code::unexpected_eof)
                                               .with_component(error_component::parser)
                                               .with_reference(ref)
                                               .with_offset(offset + object_start)
                                               .with_message("Unexpected end of file while scanning to endobj")
                                               .build());
                }

                if (end_token.type == lexer_token_type::keyword && end_token.lexeme == "endobj")
                {
                    const std::size_t end_offset = token_offset_in(source, end_token);
                    if (end_offset == std::string::npos || end_offset < object_start)
                    {
                        return std::unexpected(error_builder::create()
                                                   .with_code(error_code::invalid_object_boundary)
                                                   .with_component(error_component::parser)
                                                   .with_reference(ref)
                                                   .with_offset(offset + object_start)
                                                   .with_message("Invalid object end offset")
                                                   .build());
                    }

                    const std::size_t object_end = end_offset + end_token.lexeme.size();
                    return source.substr(object_start, object_end - object_start);
                }
            }
        }

        return std::unexpected(error_builder::create()
                                   .with_code(error_code::not_found)
                                   .with_component(error_component::parser)
                                   .with_reference(ref)
                                   .with_message("Indirect object not found")
                                   .build());
    }
}
