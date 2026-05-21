#include "core/serializer/indirect_object/default_indirect_object_serializer.hpp"

#include <algorithm>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/util/byte.hpp"
#include "core/util/text.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/object_identity.hpp"
#include "core/exceptions/exception.hpp"

namespace
{
    template <typename T>
    inline constexpr bool always_false_v = false;
}

namespace ripper::pdf::core
{
    std::vector<std::byte> default_indirect_object_serializer::serialize(const indirect_object &obj) const
    {
        const auto &ref = obj.identity().reference();

        std::vector<std::byte> out;

        const auto serialized_ref = serialize_direct_reference(ref);
        byte::append_bytes(out, serialized_ref);

        byte::append_bytes(out, object_break_character_);

        auto serialized_content = serialize_object_value(obj.content());
        byte::append_bytes(out, serialized_content);

        const std::string_view end_obj_marker = "endobj";

        byte::append_bytes(out, line_break_character_);
        byte::append_bytes(out, end_obj_marker);
        byte::append_bytes(out, line_break_character_);

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_object_value(const object &obj) const
    {
        return std::visit(
            [&](const auto &value) -> std::vector<std::byte>
            {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, null>)
                {
                    return serialize_null();
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return serialize_bool(value);
                }
                else if constexpr (std::is_same_v<T, std::int64_t>)
                {
                    return serialize_integer(value);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    return serialize_real(value);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return serialize_string(value);
                }
                else if constexpr (std::is_same_v<T, name>)
                {
                    return serialize_name(value);
                }
                else if constexpr (std::is_same_v<T, indirect_reference>)
                {
                    return serialize_indirect_reference(value);
                }
                else if constexpr (std::is_same_v<T, array>)
                {
                    return serialize_array(value);
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<dictionary>>)
                {
                    if (!value)
                        return {};
                    return serialize_dictionary(*value);
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<object_stream>>)
                {
                    if (!value)
                        return {};
                    return serialize_stream_object(*value);
                }
                else
                {
                    static_assert(always_false_v<T>, "Non-exhaustive visitor!");
                }
            },
            obj.variant());
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_bool(bool value) const
    {
        const std::string_view sv = value ? "true" : "false";

        return byte::to_bytes(sv);
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_integer(std::int64_t value) const
    {
        const auto str = std::to_string(value);

        return byte::to_bytes(str);
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_real(double value) const
    {
        std::ostringstream ss;
        ss << value;
        const auto str = ss.str();

        return byte::to_bytes(str);
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_string(const std::string &value) const
    {
        const auto escaped = text::escape_literal_string(value);

        std::vector<std::byte> out;

        byte::append_bytes(out, '(');
        byte::append_bytes(out, escaped);
        byte::append_bytes(out, ')');

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_name(const name &value) const
    {
        const auto escaped = text::escape_literal_string(value.value);

        std::vector<std::byte> out;

        byte::append_bytes(out, '/');
        byte::append_bytes(out, escaped);

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_direct_reference(const indirect_reference &value) const
    {
        const auto obj_num_str = std::to_string(value.object_number());
        const auto gen_num_str = std::to_string(value.generation());

        std::vector<std::byte> out;

        byte::append_bytes(out, obj_num_str);
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, gen_num_str);
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, "obj");

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_indirect_reference(const indirect_reference &value) const
    {
        const auto obj_num_str = std::to_string(value.object_number());
        const auto gen_num_str = std::to_string(value.generation());

        std::vector<std::byte> out;

        byte::append_bytes(out, obj_num_str);
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, gen_num_str);
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, 'R');

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_array(const array &value) const
    {
        std::vector<std::byte> out;

        byte::append_bytes(out, '[');

        for (std::size_t i = 0; i < value.size(); ++i)
        {
            if (i > 0)
                byte::append_bytes(out, ' ');

            auto item = serialize_object_value(value[i]);

            byte::append_bytes(out, item);
        }

        byte::append_bytes(out, ']');

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_dictionary(const dictionary &value) const
    {
        std::vector<std::byte> out;

        byte::append_bytes(out, '<');
        byte::append_bytes(out, '<');
        byte::append_bytes(out, object_break_character_);

        for (const auto &[key, val] : value.entries())
        {
            auto serialized_key = serialize_name(name{key});
            auto serialized_value = serialize_object_value(val);

            byte::append_bytes(out, serialized_key);
            byte::append_bytes(out, ' ');
            byte::append_bytes(out, serialized_value);
            byte::append_bytes(out, object_break_character_);
        }

        byte::append_bytes(out, '>');
        byte::append_bytes(out, '>');
        byte::append_bytes(out, line_break_character_);

        return out;
    }

    std::vector<std::byte> default_indirect_object_serializer::serialize_stream_object(const object_stream &stream_obj) const
    {
        throw not_implemented_exception{"Stream object serialization not yet implemented"};
    }
}
