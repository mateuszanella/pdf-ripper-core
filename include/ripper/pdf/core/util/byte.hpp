#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace ripper::pdf::core::byte
{
/// Convert a std::string_view to a vector of bytes.
[[nodiscard]] inline std::vector<std::byte> to_bytes(std::string_view sv)
{
    std::vector<std::byte> result(sv.size());
    std::transform(sv.begin(), sv.end(), result.begin(),
                   [](char c) { return std::byte{static_cast<unsigned char>(c)}; });

    return result;
}

/// Append the bytes of a string_view to an existing byte buffer.
inline void append_bytes(std::vector<std::byte>& out, std::string_view sv)
{
    std::transform(sv.begin(), sv.end(), std::back_inserter(out),
                   [](char c) { return std::byte{static_cast<unsigned char>(c)}; });
}

/// Append the bytes of a char to an existing byte buffer.
inline void append_bytes(std::vector<std::byte>& out, const char& c)
{
    out.push_back(std::byte{static_cast<unsigned char>(c)});
}

/// Append the bytes of a byte vector to an existing byte buffer.
inline void append_bytes(std::vector<std::byte>& out, const std::vector<std::byte>& in)
{
    out.insert(out.end(), in.begin(), in.end());
}

/// Reinterpret a std::byte pointer as a const char pointer.
/// The returned pointer is NOT null-terminated; the caller must provide an explicit size
/// when constructing a string_view from it (i.e. `std::string_view{as_chars(p), n}`).
/// This is safe because bytes read from a file are interpreted as chars.
[[nodiscard]] inline const char* as_chars(const std::byte* data) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const char*>(data);
}
} // namespace ripper::pdf::core::byte
