#include "ripper/pdf/core/parser/object_stream_parser.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/helpers/indirect_object.hpp"
#include "ripper/pdf/core/document/object/helpers/object_identity.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"
#include "ripper/pdf/core/parser/value_parsing.hpp"
#include "ripper/pdf/core/util/byte.hpp"

#include <charconv>

namespace ripper::pdf::core
{

std::vector<indirect_object> object_stream_parser::parse(document& doc,
                                                         const stream_object& stream_obj)
{
    const auto& dict = stream_obj.dictionary();

    const auto* type = dict.get_name("Type");
    if (type == nullptr || type->value != "ObjStm")
        throw parse_exception{"Object stream must have /Type /ObjStm"};

    const auto content = stream_obj.raw();

    const auto* n_ptr = dict.get_number("N");
    if (n_ptr == nullptr || n_ptr->as_integer() <= 0)
        throw parse_exception{"Object stream missing required /N"};

    const auto* first_ptr = dict.get_number("First");
    if (first_ptr == nullptr || first_ptr->as_integer() < 0)
        throw parse_exception{"Object stream missing required /First"};

    const auto n = static_cast<std::uint32_t>(n_ptr->as_integer());
    const auto first = static_cast<std::size_t>(first_ptr->as_integer());

    // Parse the header: N pairs of (object_number byte_offset)
    std::vector<std::pair<std::uint32_t, std::size_t>> entries;
    entries.reserve(n);

    std::size_t pos = 0;
    for (std::uint32_t i = 0; i < n; ++i)
    {
        pos = byte::skip_whitespace(content, pos, first);

        // Parse object number
        auto obj_num_start = pos;
        pos = byte::skip_non_whitespace(content, pos, first);

        std::string obj_num_str;
        obj_num_str.reserve(pos - obj_num_start);
        for (std::size_t j = obj_num_start; j < pos; ++j)
            obj_num_str += static_cast<char>(content[j]);

        std::uint32_t obj_num = 0;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr1, ec1] =
            std::from_chars(obj_num_str.data(), obj_num_str.data() + obj_num_str.size(), obj_num);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        if (ec1 != std::errc{})
            throw parse_exception{"Invalid object number in object stream header"};

        pos = byte::skip_whitespace(content, pos, first);

        // Parse byte offset
        auto offset_start = pos;
        pos = byte::skip_non_whitespace(content, pos, first);

        std::string offset_str;
        offset_str.reserve(pos - offset_start);
        for (std::size_t j = offset_start; j < pos; ++j)
            offset_str += static_cast<char>(content[j]);

        std::size_t byte_offset = 0;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr2, ec2] =
            std::from_chars(offset_str.data(), offset_str.data() + offset_str.size(), byte_offset);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        if (ec2 != std::errc{})
            throw parse_exception{"Invalid byte offset in object stream header"};

        entries.emplace_back(obj_num, first + byte_offset);
    }

    // Parse each object from the body
    std::vector<indirect_object> result;
    result.reserve(n);

    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        const auto [obj_num, obj_offset] = entries[i];

        // Determine the end of this object's data
        const std::size_t obj_end =
            (i + 1 < entries.size()) ? entries[i + 1].second : content.size();

        if (obj_offset >= content.size() || obj_end > content.size())
            throw parse_exception{"Object stream byte offset out of bounds"};

        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto* obj_start = reinterpret_cast<const char*>(content.data() + obj_offset);
        const auto obj_size = obj_end - obj_offset;
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

        std::string_view obj_sv{obj_start, obj_size};

        // Parse the object value
        pdf_lexer lexer{obj_sv};
        auto obj_value = parse_value(lexer);

        if (obj_value.is_stream())
            throw parse_exception{
                "Object stream shall not contain stream objects (ISO 32000-1 §7.6.3)"};

        // Create indirect_object with identity
        const indirect_reference ref{obj_num, 0};
        result.emplace_back(object_identity{&doc, ref}, std::move(obj_value));
    }

    return result;
}

} // namespace ripper::pdf::core
