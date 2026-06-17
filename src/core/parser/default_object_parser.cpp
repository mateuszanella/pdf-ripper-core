#include "ripper/pdf/core/parser/default_object_parser.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/object/stream.hpp"
#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"
#include "ripper/pdf/core/parser/value_parsing.hpp"

#include <cstring>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
indirect_object default_object_parser::parse(document& doc, indirect_reference ref,
                                             std::string_view content_sv) const
{
    pdf_lexer lexer{content_sv};

    // Skip the `N G obj` header. The reference is already known from the xref.
    for (int i = 0; i < 3; ++i)
    {
        (void)lexer.next();
    }

    // Parse the content stream as a PDF direct value (primitive value, listed on object.hpp).
    auto content = parse_value(lexer);

    // After parsing the actual content and obtaining the proper object type, we must
    // now check if the content contains a `stream` keyword. By default, we only allow
    // dictionaries to contain stream content, as other types do not support it.
    if (content.is_dictionary())
    {
        const auto* dict = content.as_dictionary();

        auto peek_result = lexer.peek();

        if (peek_result.type == lexer_token_type::keyword && peek_result.lexeme == "stream")
        {
            auto stream_tok = lexer.next();

            // Stream content begins after the `stream` keyword and its mandatory
            // line ending (either \r\n or \n per the PDF spec, section 7.3.8.1).

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto* pos = stream_tok.lexeme.data() + stream_tok.lexeme.size();

            const auto* data_end = content_sv.data() + content_sv.size();

            if (pos < data_end && *pos == '\r')
                ++pos;
            if (pos < data_end && *pos == '\n')
                ++pos;
            // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

            const auto stream_bytes_start_offset =
                static_cast<std::size_t>(pos - content_sv.data());

            // Determine the stream content boundary.
            //
            // Per PDF spec §7.3.8, /Length is the authoritative source. We cross-
            // validate it against the actual "endstream" position and fall back
            // to searching for "endstream" when they disagree (malformed /Length).
            std::size_t end_of_stream = std::string_view::npos;

            const std::int64_t* length_ptr = dict->get_integer("Length");
            if (length_ptr != nullptr && *length_ptr >= 0)
            {
                const auto length = static_cast<std::size_t>(*length_ptr);
                const auto length_end = stream_bytes_start_offset + length;

                std::size_t check = length_end;
                while (check < content_sv.size() &&
                       (content_sv[check] == '\r' || content_sv[check] == '\n' ||
                        content_sv[check] == ' '))
                {
                    ++check;
                }

                if (check + 9 <= content_sv.size() && content_sv.substr(check, 9) == "endstream")
                {
                    end_of_stream = length_end;
                }
            }

            if (end_of_stream == std::string_view::npos)
            {
                end_of_stream = content_sv.rfind("endstream");
                if (end_of_stream != std::string_view::npos)
                {
                    while (end_of_stream > stream_bytes_start_offset &&
                           (content_sv[end_of_stream - 1] == '\r' ||
                            content_sv[end_of_stream - 1] == '\n'))
                    {
                        --end_of_stream;
                    }
                }
                else
                {
                    end_of_stream = content_sv.size();
                }
            }

            std::vector<std::byte> bytes(end_of_stream - stream_bytes_start_offset);

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::memcpy(bytes.data(), content_sv.data() + stream_bytes_start_offset, bytes.size());

            stream parsed_stream = stream{std::move(bytes)};

            return indirect_object{
                object_identity{&doc, ref},
                object{object_stream{std::move(*dict), std::move(parsed_stream)}}};
        }
    }

    return indirect_object{object_identity{&doc, ref}, std::move(content)};
}
} // namespace ripper::pdf::core
