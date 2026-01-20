#pragma once

#include <charconv>
#include <cctype>
#include <optional>
#include <string_view>
#include <system_error>

namespace Ripper::Core::Text
{
    [[nodiscard]] constexpr std::string_view StripLineEndings(std::string_view s) noexcept
    {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        {
            s.remove_suffix(1);
        }

        return s;
    }

    [[nodiscard]] inline std::string_view TrimAscii(std::string_view s) noexcept
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

    [[nodiscard]] inline bool StartsWithToken(std::string_view line, std::string_view token) noexcept
    {
        line = TrimAscii(StripLineEndings(line));

        if (line.size() < token.size())
        {
            return false;
        }

        return line.starts_with(token);
    }

    [[nodiscard]] inline std::optional<std::size_t> ParseSizeT(std::string_view s) noexcept
    {
        s = TrimAscii(StripLineEndings(s));
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
}
