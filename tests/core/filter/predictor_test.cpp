#include "ripper/pdf/core/document/object/dictionary_object.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/filter_manager.hpp"
#include "ripper/pdf/core/filter/flate_decode_filter.hpp"
#include "ripper/pdf/core/filter/predictor.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{

std::vector<std::byte> bytes(std::string_view sv)
{
    std::vector<std::byte> out;
    out.reserve(sv.size());
    for (const char c : sv)
        out.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));
    return out;
}

[[nodiscard]] std::uint8_t picture(std::size_t i) noexcept
{
    return static_cast<std::uint8_t>((i * 37 + 11) & 0xFF);
}

/// Rebuild a PNG-predicted byte buffer by applying `tag` across every row.
/// Mirrors the encoder-side transform the decoder must invert.
std::vector<std::byte> png_encode(const std::vector<std::vector<std::uint8_t>>& rows,
                                  std::size_t bytes_per_pixel, std::uint8_t tag)
{
    std::vector<std::byte> out;
    std::vector<std::uint8_t> previous(rows.front().size(), 0);

    for (const auto& row : rows)
    {
        std::vector<std::uint8_t> filtered(row.size());
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            const std::uint8_t left = i >= bytes_per_pixel ? row[i - bytes_per_pixel] : 0;
            const std::uint8_t up = previous[i];
            const std::uint8_t upper_left =
                i >= bytes_per_pixel ? previous[i - bytes_per_pixel] : 0;

            unsigned int predictor = 0;
            switch (tag)
            {
                case 1:
                    predictor = left;
                    break;
                case 2:
                    predictor = up;
                    break;
                case 3:
                    predictor = (static_cast<unsigned int>(left) + up) / 2U;
                    break;
                case 4:
                {
                    const auto p = static_cast<int>(left) + up - upper_left;
                    const auto pa = p - static_cast<int>(left);
                    const auto pb = p - static_cast<int>(up);
                    const auto pc = p - static_cast<int>(upper_left);
                    const auto a = pa < 0 ? -pa : pa;
                    const auto bb = pb < 0 ? -pb : pb;
                    const auto c = pc < 0 ? -pc : pc;
                    predictor = (a <= bb && a <= c) ? left : ((bb <= c) ? up : upper_left);
                    break;
                }
                default:
                    break;
            }

            filtered[i] = static_cast<std::uint8_t>(row[i] - predictor);
        }

        out.push_back(std::byte{tag});
        for (const auto v : filtered)
            out.push_back(static_cast<std::byte>(v));

        previous = row;
    }

    return out;
}

[[nodiscard]] dictionary_object png_params(std::uint64_t columns, std::uint64_t predictor,
                                           std::uint64_t colors = 1, std::uint64_t bpc = 8)
{
    dictionary_object params;
    params.set("Predictor", object{static_cast<std::int64_t>(predictor)});
    params.set("Columns", object{static_cast<std::int64_t>(columns)});
    params.set("Colors", object{static_cast<std::int64_t>(colors)});
    params.set("BitsPerComponent", object{static_cast<std::int64_t>(bpc)});
    return params;
}

} // namespace

TEST_CASE("predictor has_predictor", "[filter][predictor]")
{
    REQUIRE_FALSE(has_predictor(nullptr));

    dictionary_object none;
    none.set("Predictor", object{std::int64_t{1}});
    REQUIRE_FALSE(has_predictor(&none));

    dictionary_object png;
    png.set("Predictor", object{std::int64_t{12}});
    REQUIRE(has_predictor(&png));

    dictionary_object tiff;
    tiff.set("Predictor", object{std::int64_t{2}});
    REQUIRE(has_predictor(&tiff));
}

TEST_CASE("predictor identity returns input unchanged", "[filter][predictor]")
{
    auto input = bytes("hello world");
    dictionary_object params;
    params.set("Predictor", object{std::int64_t{1}});
    REQUIRE(apply_predictor(input, params) == input);
}

TEST_CASE("predictor TIFF reverses horizontal differencing", "[filter][predictor]")
{
    const std::vector<std::uint8_t> row_a{10, 20, 30, 40, 50, 60};
    const std::vector<std::uint8_t> row_b{5, 6, 7, 8, 9, 10};

    std::vector<std::byte> encoded;
    for (const auto* row : {&row_a, &row_b})
    {
        for (std::size_t i = 0; i < row->size(); ++i)
        {
            const std::uint8_t left = i >= 2 ? (*row)[i - 2] : 0;
            encoded.push_back(static_cast<std::byte>(static_cast<std::uint8_t>((*row)[i] - left)));
        }
    }

    dictionary_object params = png_params(3, 2, 2);
    const auto decoded = apply_predictor(encoded, params);

    REQUIRE(decoded.size() == 12);
    for (std::size_t i = 0; i < row_a.size(); ++i)
        REQUIRE(decoded[i] == static_cast<std::byte>(row_a[i]));
    for (std::size_t i = 0; i < row_b.size(); ++i)
        REQUIRE(decoded[row_a.size() + i] == static_cast<std::byte>(row_b[i]));
}

TEST_CASE("predictor TIFF rejects non-8-bit samples", "[filter][predictor][error]")
{
    const auto input = bytes("abc");
    auto params = png_params(1, 2, 1);
    params.set("BitsPerComponent", object{std::int64_t{16}});
    REQUIRE_THROWS_AS(apply_predictor(input, params), parse_exception);
}

TEST_CASE("predictor PNG round-trips all filter types", "[filter][predictor]")
{
    const std::size_t columns = 4;
    const std::size_t colors = 1;
    const std::size_t bpc = 8;
    const std::size_t row_length = columns * colors * bpc / 8;

    const std::size_t row_count = 3;
    std::vector<std::vector<std::uint8_t>> rows;
    for (std::size_t r = 0; r < row_count; ++r)
    {
        std::vector<std::uint8_t> row(row_length);
        for (std::size_t i = 0; i < row_length; ++i)
            row[i] = picture(r * row_length + i);
        rows.push_back(std::move(row));
    }

    for (std::uint8_t tag :
         {std::uint8_t{0}, std::uint8_t{1}, std::uint8_t{2}, std::uint8_t{3}, std::uint8_t{4}})
    {
        SECTION("PNG filter type " + std::to_string(tag))
        {
            const auto encoded = png_encode(rows, 1, tag);
            auto params = png_params(columns, static_cast<std::uint64_t>(10 + tag));
            const auto decoded = apply_predictor(encoded, params);

            REQUIRE(decoded.size() == row_count * row_length);
            for (std::size_t i = 0; i < rows.size(); ++i)
                for (std::size_t j = 0; j < rows[i].size(); ++j)
                    REQUIRE(decoded[i * row_length + j] == static_cast<std::byte>(rows[i][j]));
        }
    }
}

TEST_CASE("predictor PNG rejects unknown filter type", "[filter][predictor][error]")
{
    auto input = bytes("abc");
    input.insert(input.begin(), std::byte{9});
    auto params = png_params(3, 12);
    REQUIRE_THROWS_AS(apply_predictor(input, params), parse_exception);
}

TEST_CASE("predictor PNG rejects truncated rows", "[filter][predictor][error]")
{
    auto input = bytes("abc"); // one filter byte + two bytes, but Columns=4 -> needs 5
    auto params = png_params(4, 12);
    REQUIRE_THROWS_AS(apply_predictor(input, params), parse_exception);
}

TEST_CASE("predictor rejects unsupported value", "[filter][predictor][error]")
{
    auto params = png_params(1, 99);
    REQUIRE_THROWS_AS(apply_predictor(bytes("x"), params), parse_exception);
}

TEST_CASE("filter_manager decode applies predictor after FlateDecode",
          "[filter][manager][predictor]")
{
    const std::size_t columns = 5;
    const std::uint8_t tag = 1; // Sub
    const std::size_t row_length = 5;
    const std::size_t row_count = 4;

    std::vector<std::vector<std::uint8_t>> rows;
    for (std::size_t r = 0; r < row_count; ++r)
    {
        std::vector<std::uint8_t> row(row_length);
        for (std::size_t i = 0; i < row_length; ++i)
            row[i] = picture(r * row_length + i);
        rows.push_back(std::move(row));
    }

    const auto predicted = png_encode(rows, 1, tag);

    flate_decode_filter flate;
    const auto compressed = flate.encode(predicted);

    dictionary_object dict;
    dict.set("Filter", object{name_object{"FlateDecode"}});
    dict.set("DecodeParms", object{png_params(columns, static_cast<std::uint64_t>(10 + tag))});

    const auto decoded = filter_manager::decode(dict, compressed);
    REQUIRE(decoded.size() == row_count * row_length);
    for (std::size_t i = 0; i < rows.size(); ++i)
        for (std::size_t j = 0; j < rows[i].size(); ++j)
            REQUIRE(decoded[i * row_length + j] == static_cast<std::byte>(rows[i][j]));
}

} // namespace ripper::pdf::core