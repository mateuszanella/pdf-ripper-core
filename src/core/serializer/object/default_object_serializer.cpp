#include "core/serializer/object/default_object_serializer.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "core/util/byte.hpp"
#include "core/util/text.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace
{
    template <typename T>
    inline constexpr bool always_false_v = false;
}

namespace ripper::pdf::core
{
    std::vector<std::byte> default_object_serializer::serialize(const object &obj) const
    {
        return std::visit(
            [&](const auto &value) -> std::vector<std::byte>
            {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, null>)
                    return serialize_null();
                else if constexpr (std::is_same_v<T, bool>)
                    return serialize_bool(value);
                else if constexpr (std::is_same_v<T, std::int64_t>)
                    return serialize_integer(value);
                else if constexpr (std::is_same_v<T, double>)
                    return serialize_real(value);
                else if constexpr (std::is_same_v<T, std::string>)
                    return serialize_string(value);
                else if constexpr (std::is_same_v<T, name>)
                    return serialize_name(value);
                else if constexpr (std::is_same_v<T, indirect_reference>)
                    return serialize_indirect_reference(value);
                else if constexpr (std::is_same_v<T, array>)
                    return serialize_array(value);
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
                    static_assert(always_false_v<T>, "Non-exhaustive visitor!");
            },
            obj.variant());
    }

    std::vector<std::byte> default_object_serializer::serialize_bool(bool value) const
    {
        return byte::to_bytes(value ? std::string_view{"true"} : std::string_view{"false"});
    }

    std::vector<std::byte> default_object_serializer::serialize_integer(std::int64_t value) const
    {
        return byte::to_bytes(std::to_string(value));
    }

    std::vector<std::byte> default_object_serializer::serialize_real(double value) const
    {
        std::ostringstream ss;
        ss << value;
        return byte::to_bytes(ss.str());
    }

    std::vector<std::byte> default_object_serializer::serialize_string(const std::string &value) const
    {
        const auto escaped = text::escape_literal_string(value);

        std::vector<std::byte> out;
        byte::append_bytes(out, '(');
        byte::append_bytes(out, escaped);
        byte::append_bytes(out, ')');
        return out;
    }

    std::vector<std::byte> default_object_serializer::serialize_name(const name &value) const
    {
        const auto escaped = text::escape_literal_string(value.value);

        std::vector<std::byte> out;
        byte::append_bytes(out, '/');
        byte::append_bytes(out, escaped);
        return out;
    }

    std::vector<std::byte> default_object_serializer::serialize_indirect_reference(const indirect_reference &value) const
    {
        std::vector<std::byte> out;
        byte::append_bytes(out, std::to_string(value.object_number()));
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, std::to_string(value.generation()));
        byte::append_bytes(out, ' ');
        byte::append_bytes(out, 'R');
        return out;
    }

    std::vector<std::byte> default_object_serializer::serialize_array(const array &value) const
    {
        std::vector<std::byte> out;
        byte::append_bytes(out, '[');
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            if (i > 0)
                byte::append_bytes(out, ' ');
            byte::append_bytes(out, serialize(value[i]));
        }
        byte::append_bytes(out, ']');
        return out;
    }

    std::vector<std::byte> default_object_serializer::serialize_dictionary(const dictionary &value) const
    {
        std::vector<std::byte> out;
        byte::append_bytes(out, '<');
        byte::append_bytes(out, '<');
        byte::append_bytes(out, object_break_character_);
        for (const auto &[key, val] : value.entries())
        {
            byte::append_bytes(out, serialize_name(name{key}));
            byte::append_bytes(out, ' ');
            byte::append_bytes(out, serialize(val));
            byte::append_bytes(out, object_break_character_);
        }
        byte::append_bytes(out, '>');
        byte::append_bytes(out, '>');
        byte::append_bytes(out, line_break_character_);
        return out;
    }

    std::vector<std::byte> default_object_serializer::serialize_stream_object(const object_stream &) const
    {
        throw not_implemented_exception{"Stream object serialization not yet implemented"};
    }
}
