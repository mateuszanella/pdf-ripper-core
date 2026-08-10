#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/lzw_decode_filter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{

std::vector<std::byte> b(std::string_view sv)
{
    std::vector<std::byte> v;
    v.reserve(sv.size());
    for (char c : sv)
        v.push_back(static_cast<std::byte>(c));
    return v;
}

std::vector<std::byte> pack_bits(const std::vector<int>& codes, int code_size)
{
    std::vector<std::byte> bytes;
    int bit_pos = 0;

    for (int code : codes)
    {
        for (int i = code_size - 1; i >= 0; --i)
        {
            const int byte_idx = bit_pos / 8;
            while (byte_idx >= static_cast<int>(bytes.size()))
                bytes.push_back(std::byte(0));

            const int bit_idx = 7 - (bit_pos % 8);
            if ((code >> i) & 1)
                bytes[static_cast<std::size_t>(byte_idx)] |= std::byte(1 << bit_idx);

            ++bit_pos;
        }
    }

    return bytes;
}

} // namespace

TEST_CASE("lzw_decode_filter round-trip", "[filter][lzw]")
{
    lzw_decode_filter filter;

    SECTION("empty input")
    {
        std::vector<std::byte> input;
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded.empty());
    }

    SECTION("single byte")
    {
        auto input = b("A");
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }

    SECTION("hello")
    {
        auto input = b("hello");
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }

    SECTION("repeated data compresses well")
    {
        std::string data(1000, 'A');
        auto input = b(data);
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
        REQUIRE(encoded.size() < input.size());
    }

    SECTION("binary data round-trips")
    {
        std::vector<std::byte> input(256);
        for (int i = 0; i < 256; ++i)
            input[static_cast<std::size_t>(i)] = static_cast<std::byte>(i);
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }

    SECTION("large data round-trips")
    {
        std::vector<std::byte> input(1024 * 16);
        for (std::size_t i = 0; i < input.size(); ++i)
            input[i] = static_cast<std::byte>(i % 251);
        auto encoded = filter.encode(input);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == input);
    }
}

TEST_CASE("lzw_decode_filter round-trip with EarlyChange=0", "[filter][lzw]")
{
    lzw_decode_filter filter;

    dictionary_object params;
    params.set("EarlyChange", object{number_object{std::int64_t{0}}});

    auto input = b("hello");
    auto encoded = filter.encode(input, &params);
    auto decoded = filter.decode(encoded, &params);
    REQUIRE(decoded == input);
}

TEST_CASE("lzw_decode_filter decode hand-crafted", "[filter][lzw]")
{
    lzw_decode_filter filter;

    SECTION("single byte")
    {
        std::vector<int> codes = {256, 65, 257};
        auto encoded = pack_bits(codes, 9);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded.size() == 1);
        REQUIRE(decoded[0] == std::byte{'A'});
    }

    SECTION("hello")
    {
        std::vector<int> codes;
        codes.push_back(256);
        codes.push_back('h');
        codes.push_back('e');
        codes.push_back('l');
        codes.push_back('l');
        codes.push_back('o');
        codes.push_back(257);
        auto encoded = pack_bits(codes, 9);
        auto decoded = filter.decode(encoded);
        REQUIRE(decoded == b("hello"));
    }
}

} // namespace ripper::pdf::core
