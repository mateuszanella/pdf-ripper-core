#include "core/parser/default_object_parser.hpp"

#include "core/document.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/stream.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"
#include "core/parser/value_parsing.hpp"

#include <cstring>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
indirect_object default_object_parser::parse(document& doc, indirect_reference ref,
                                             std::string_view content_sv, bool preload_stream) const
{
    pdf_lexer lexer{content_sv};

    // Skip the `N G obj` header. The reference is already known from the xref.
    for (int i = 0; i < 3; ++i)
    {
        (void)lexer.next();
    }

    // Parse the content as any PDF direct value (primitive, array, dictionary, etc.).
    auto content = parse_value(lexer);

    // A content stream is only possible when the content is a dictionary.
    if (content.is_dictionary())
    {
        const auto* dict_ptr = content.as_dictionary();
        auto peek_result = lexer.peek();

        /// TODO: implement streams correctly and also add a way to create a proper deferred stream.

        if (peek_result.type == lexer_token_type::keyword && peek_result.lexeme == "stream")
        {
            auto stream_tok = lexer.next();

            // Stream content begins after the `stream` keyword and its mandatory
            // line ending (either \r\n or \n per the PDF spec, section 7.3.8.1).
            const char* kw_end = stream_tok.lexeme.data() + stream_tok.lexeme.size();
            const char* content_end = content_sv.data() + content_sv.size();

            const char* stream_start = kw_end;
            if (stream_start < content_end && *stream_start == '\r')
                ++stream_start;
            if (stream_start < content_end && *stream_start == '\n')
                ++stream_start;

            // Locate `endstream` to bound the raw stream bytes.
            const std::string_view remainder{stream_start,
                                             static_cast<std::size_t>(content_end - stream_start)};

            const auto endstream_pos = remainder.find("endstream");

            std::vector<std::byte> bytes;
            std::size_t payload_size = 0;
            if (endstream_pos != std::string_view::npos)
            {
                // Trim the mandatory line ending before `endstream`.
                std::size_t len = endstream_pos;
                if (len > 0 && remainder[len - 1] == '\n')
                    --len;
                if (len > 0 && remainder[len - 1] == '\r')
                    --len;

                payload_size = len;

                if (preload_stream)
                {
                    bytes.resize(len);
                    std::memcpy(bytes.data(), stream_start, len);
                }
            }

            dictionary stream_dict{};
            if (dict_ptr)
                stream_dict = *dict_ptr;

            const auto* length = stream_dict.get_integer("Length");
            const std::size_t expected_size =
                length ? static_cast<std::size_t>(*length) : payload_size;

            stream parsed_stream =
                preload_stream ? stream{std::move(bytes)} : stream::deferred(expected_size);

            return indirect_object{
                object_identity{&doc, ref},
                object{object_stream{std::move(stream_dict), std::move(parsed_stream)}}};
        }
    }

    return indirect_object{object_identity{&doc, ref}, std::move(content)};
}
} // namespace ripper::pdf::core
