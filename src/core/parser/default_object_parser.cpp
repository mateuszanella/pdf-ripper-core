#include "core/parser/default_object_parser.hpp"

#include <cstring>
#include <string_view>
#include <vector>

#include "core/document.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/stream.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/value_parsing.hpp"

namespace ripper::core
{
    std::expected<object, error> default_object_parser::parse(
        document &doc,
        indirect_reference ref,
        std::string_view content_sv) const
    {
        pdf_lexer lexer{content_sv};

        // Skip the `N G obj` header. The reference is already known from the xref.
        for (int i = 0; i < 3; ++i)
        {
            auto tok = lexer.next();
            if (!tok)
                return std::unexpected(tok.error());
        }

        // Parse the content as any PDF direct value (primitive, array, dictionary, etc.).
        auto content = parse_value(lexer);
        if (!content)
            return std::unexpected(content.error());

        // A content stream is only possible when the content is a dictionary.
        if (content->is_dictionary())
        {
            auto peek_result = lexer.peek();
            if (!peek_result)
                return std::unexpected(peek_result.error());

            if (peek_result->type == lexer_token_type::keyword && peek_result->lexeme == "stream")
            {
                auto stream_tok = *lexer.next();

                // Stream content begins after the `stream` keyword and its mandatory
                // line ending (either \r\n or \n per the PDF spec, section 7.3.8.1).
                const char *kw_end = stream_tok.lexeme.data() + stream_tok.lexeme.size();
                const char *content_end = content_sv.data() + content_sv.size();

                const char *stream_start = kw_end;
                if (stream_start < content_end && *stream_start == '\r')
                    ++stream_start;
                if (stream_start < content_end && *stream_start == '\n')
                    ++stream_start;

                // Locate `endstream` to bound the raw stream bytes.
                const std::string_view remainder{stream_start,
                                                 static_cast<std::size_t>(content_end - stream_start)};

                const auto endstream_pos = remainder.find("endstream");

                std::vector<std::byte> bytes;
                if (endstream_pos != std::string_view::npos)
                {
                    // Trim the mandatory line ending before `endstream`.
                    std::size_t len = endstream_pos;
                    if (len > 0 && remainder[len - 1] == '\n')
                        --len;
                    if (len > 0 && remainder[len - 1] == '\r')
                        --len;

                    bytes.resize(len);
                    std::memcpy(bytes.data(), stream_start, len);
                }

                return object{
                    indirect_object{doc, ref},
                    std::move(*content),
                    stream{std::move(bytes)}};
            }
        }

        return object{indirect_object{doc, ref}, std::move(*content)};
    }
}
