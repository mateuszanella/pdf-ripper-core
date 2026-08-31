#include "ripper/pdf/core/filter/ascii_hex_decode_filter.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
namespace
{

[[nodiscard]] int hex_value(std::byte b) noexcept
{
    const auto c = static_cast<unsigned char>(b);
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

[[nodiscard]] bool is_whitespace(std::byte b) noexcept
{
    const auto c = static_cast<unsigned char>(b);
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\0';
}

} // namespace

std::vector<std::byte> ascii_hex_decode_filter::decode(std::span<const std::byte> input,
                                                       const dictionary_object* /*params*/) const
{
    std::vector<std::byte> output;
    output.reserve(input.size() / 2);

    int high = -1;

    for (auto b : input)
    {
        if (b == std::byte{'>'})
            break;

        if (is_whitespace(b))
            continue;

        const int val = hex_value(b);
        if (val < 0)
            throw parse_exception{"Invalid hex digit in ASCIIHexDecode stream"};

        if (high < 0)
        {
            high = val;
        }
        else
        {
            output.push_back(static_cast<std::byte>(static_cast<unsigned char>((high << 4) | val)));
            high = -1;
        }
    }

    if (high >= 0)
        output.push_back(static_cast<std::byte>(static_cast<unsigned char>(high << 4)));

    return output;
}

std::vector<std::byte> ascii_hex_decode_filter::encode(std::span<const std::byte> input,
                                                       const dictionary_object* /*params*/) const
{
    std::vector<std::byte> output;
    output.reserve(input.size() * 2 + 1);

    for (auto b : input)
    {
        const auto v = static_cast<unsigned char>(b);
        const auto hi = static_cast<unsigned char>(v >> 4);
        const auto lo = static_cast<unsigned char>(v & 0x0F);
        output.push_back(static_cast<std::byte>(hi < 10
                                                    ? static_cast<unsigned char>('0' + hi)
                                                    : static_cast<unsigned char>('A' + hi - 10)));
        output.push_back(static_cast<std::byte>(lo < 10
                                                    ? static_cast<unsigned char>('0' + lo)
                                                    : static_cast<unsigned char>('A' + lo - 10)));
    }

    output.push_back(std::byte{'>'});
    return output;
}

} // namespace ripper::pdf::core
