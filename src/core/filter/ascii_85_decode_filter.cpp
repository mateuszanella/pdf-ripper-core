#include "ripper/pdf/core/filter/ascii_85_decode_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace ripper::pdf::core
{
namespace
{

[[nodiscard]] bool is_whitespace(std::byte b) noexcept
{
    const auto c = static_cast<unsigned char>(b);
    return c <= ' ' || c == 0x7F;
}

[[nodiscard]] constexpr std::byte encode_char(std::uint32_t code) noexcept
{
    return static_cast<std::byte>(static_cast<unsigned char>('!' + code));
}

[[nodiscard]] constexpr int decode_char(std::byte b) noexcept
{
    const auto c = static_cast<unsigned char>(b);
    return static_cast<int>(c - '!');
}

} // namespace

std::vector<std::byte> ascii_85_decode_filter::decode(std::span<const std::byte> input,
                                                      const dictionary_object* /*params*/) const
{
    std::vector<std::byte> output;
    output.reserve(input.size());

    std::array<int, 5> group{};
    int group_size = 0;

    for (auto b : input)
    {
        if (b == std::byte{'z'} && group_size == 0)
        {
            for (int i = 0; i < 4; ++i)
                output.push_back(std::byte{0});
            continue;
        }

        if (b == std::byte{'~'})
            break;

        if (is_whitespace(b))
            continue;

        const auto c = static_cast<unsigned char>(b);
        if (c < '!' || c > 'u')
            throw parse_exception{"Invalid character in ASCII85Decode stream"};

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        group[static_cast<std::size_t>(group_size)] = decode_char(b);
        ++group_size;

        if (group_size == 5)
        {
            auto v0 = static_cast<std::uint32_t>(group[0]);
            auto v1 = static_cast<std::uint32_t>(group[1]);
            auto v2 = static_cast<std::uint32_t>(group[2]);
            auto v3 = static_cast<std::uint32_t>(group[3]);
            auto v4 = static_cast<std::uint32_t>(group[4]);
            std::uint32_t value = ((((v0 * 85 + v1) * 85 + v2) * 85 + v3) * 85 + v4);

            output.push_back(
                static_cast<std::byte>(static_cast<unsigned char>((value >> 24) & 0xFF)));
            output.push_back(
                static_cast<std::byte>(static_cast<unsigned char>((value >> 16) & 0xFF)));
            output.push_back(
                static_cast<std::byte>(static_cast<unsigned char>((value >> 8) & 0xFF)));
            output.push_back(static_cast<std::byte>(static_cast<unsigned char>(value & 0xFF)));

            group_size = 0;
        }
    }

    if (group_size > 0)
    {
        for (int i = group_size; i < 5; ++i)
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            group[static_cast<std::size_t>(i)] = 84;

        auto v0 = static_cast<std::uint32_t>(group[0]);
        auto v1 = static_cast<std::uint32_t>(group[1]);
        auto v2 = static_cast<std::uint32_t>(group[2]);
        auto v3 = static_cast<std::uint32_t>(group[3]);
        auto v4 = static_cast<std::uint32_t>(group[4]);
        std::uint32_t value = ((((v0 * 85 + v1) * 85 + v2) * 85 + v3) * 85 + v4);

        for (int i = 0; i < group_size - 1; ++i)
            output.push_back(
                static_cast<std::byte>(static_cast<unsigned char>((value >> (24 - 8 * i)) & 0xFF)));
    }

    return output;
}

std::vector<std::byte> ascii_85_decode_filter::encode(std::span<const std::byte> input,
                                                      const dictionary_object* /*params*/) const
{
    std::vector<std::byte> output;
    output.reserve(input.size() + input.size() / 4 + 2);

    std::size_t i = 0;
    const std::size_t size = input.size();

    while (i + 4 <= size)
    {
        std::uint32_t value =
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i])) << 24) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 1])) << 16) |
            (static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 2])) << 8) |
            static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + 3]));
        i += 4;

        if (value == 0)
        {
            output.push_back(std::byte{'z'});
            continue;
        }

        std::array<std::byte, 5> encoded{};
        encoded[4] = encode_char(value % 85);
        value /= 85;
        encoded[3] = encode_char(value % 85);
        value /= 85;
        encoded[2] = encode_char(value % 85);
        value /= 85;
        encoded[1] = encode_char(value % 85);
        value /= 85;
        encoded[0] = encode_char(value);

        for (const auto c : encoded)
            output.push_back(c);
    }

    const std::size_t remaining = size - i;
    if (remaining > 0)
    {
        std::uint32_t value = 0;
        for (std::size_t j = 0; j < remaining; ++j)
            value =
                (value << 8) | static_cast<std::uint32_t>(static_cast<unsigned char>(input[i + j]));
        value <<= 8 * (4 - remaining);

        std::array<std::byte, 5> encoded{};
        encoded[4] = encode_char(value % 85);
        value /= 85;
        encoded[3] = encode_char(value % 85);
        value /= 85;
        encoded[2] = encode_char(value % 85);
        value /= 85;
        encoded[1] = encode_char(value % 85);
        value /= 85;
        encoded[0] = encode_char(value);

        for (std::size_t j = 0; j < remaining + 1; ++j)
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            output.push_back(encoded[j]);
    }

    output.push_back(std::byte{'~'});
    output.push_back(std::byte{'>'});
    return output;
}

} // namespace ripper::pdf::core
