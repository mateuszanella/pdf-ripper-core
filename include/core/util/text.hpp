#pragma once

#include <charconv>
#include <cctype>
#include <optional>
#include <string_view>
#include <system_error>

namespace ripper::core::text
{
    [[nodiscard]] constexpr std::string_view strip_line_endings(std::string_view s) noexcept
    {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        {
            s.remove_suffix(1);
        }

        return s;
    }

    [[nodiscard]] inline std::string_view trim_ascii(std::string_view s) noexcept
    {
        while (!s.empty())
        {
            const unsigned char c = static_cast<unsigned char>(s.front());
            if (!std::isspace(c))
            {
                break;
            }

            s.remove_prefix(1);
        }

        while (!s.empty())
        {
            const unsigned char c = static_cast<unsigned char>(s.back());
            if (!std::isspace(c))
            {
                break;
            }

            s.remove_suffix(1);
        }

        return s;
    }

    [[nodiscard]] inline bool starts_with_token(std::string_view line, std::string_view token) noexcept
    {
        line = trim_ascii(strip_line_endings(line));

        if (line.size() < token.size())
        {
            return false;
        }

        return line.starts_with(token);
    }

    [[nodiscard]] inline std::optional<std::size_t> parse_size_t(std::string_view s) noexcept
    {
        s = trim_ascii(strip_line_endings(s));
        if (s.empty())
        {
            return std::nullopt;
        }

        std::size_t value = 0;
        const char *begin = s.data();
        const char *end = s.data() + s.size();

        auto [ptr, ec] = std::from_chars(begin, end, value);
        if (ec != std::errc{})
        {
            return std::nullopt;
        }

        return value;
    }

    [[nodiscard]] inline std::optional<std::uint32_t> parse_u32(std::string_view text) noexcept
    {
        std::uint32_t value{};
        const char *first = text.data();
        const char *last = text.data() + text.size();
        auto [ptr, ec] = std::from_chars(first, last, value);

        if (ec != std::errc{} || ptr != last)
        {
            return std::nullopt;
        }

        return value;
    }

    [[nodiscard]] inline std::optional<std::uint16_t> parse_u16(std::string_view text) noexcept
    {
        const auto v32 = parse_u32(text);
        if (!v32.has_value() || *v32 > 0xFFFFu)
        {
            return std::nullopt;
        }

        return static_cast<std::uint16_t>(*v32);
    }
}
