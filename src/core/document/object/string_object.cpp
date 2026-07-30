#include "ripper/pdf/core/document/object/string_object.hpp"

namespace ripper::pdf::core
{

/// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
string_object::string_object(std::string decoded)
    : decoded_bytes_(reinterpret_cast<const std::byte*>(decoded.data()),
                     reinterpret_cast<const std::byte*>(decoded.data()) + decoded.size()),
      encoding_(encoding::utf8), form_(form::literal), original_bytes_(decoded_bytes_)
{
}
/// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

string_object::string_object(std::vector<std::byte> decoded_bytes,
                             std::vector<std::byte> original_bytes, encoding enc, form f) noexcept
    : decoded_bytes_(std::move(decoded_bytes)), original_bytes_(std::move(original_bytes)),
      encoding_(enc), form_(f)
{
}

/// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
string_object::string_object(std::string_view decoded, std::vector<std::byte> original_bytes,
                             encoding enc, form f)
    : decoded_bytes_(reinterpret_cast<const std::byte*>(decoded.data()),
                     reinterpret_cast<const std::byte*>(decoded.data()) + decoded.size()),
      original_bytes_(std::move(original_bytes)), encoding_(enc), form_(f)
{
}
/// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

const std::vector<std::byte>& string_object::decoded_bytes() const noexcept
{
    return decoded_bytes_;
}

const std::vector<std::byte>& string_object::original_bytes() const noexcept
{
    return original_bytes_;
}

string_object::encoding string_object::get_encoding() const noexcept
{
    return encoding_;
}

string_object::form string_object::get_form() const noexcept
{
    return form_;
}

bool string_object::is_hex() const noexcept
{
    return form_ == form::hex;
}

bool string_object::is_literal() const noexcept
{
    return form_ == form::literal;
}

std::string string_object::as_string() const
{
    return {reinterpret_cast<const char*>(decoded_bytes_.data()), decoded_bytes_.size()};
}

std::string_view string_object::as_string_view() const noexcept
{
    return {reinterpret_cast<const char*>(decoded_bytes_.data()), decoded_bytes_.size()};
}

bool string_object::operator==(const string_object& other) const noexcept
{
    return decoded_bytes_ == other.decoded_bytes_;
}

bool string_object::operator==(std::string_view sv) const
{
    return as_string_view() == sv;
}

} // namespace ripper::pdf::core
