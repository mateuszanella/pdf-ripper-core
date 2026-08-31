#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/ccitt_fax_decode_filter.hpp"
#include "ripper/pdf/core/filter/crypt_filter.hpp"
#include "ripper/pdf/core/filter/dct_decode_filter.hpp"
#include "ripper/pdf/core/filter/filter_manager.hpp"
#include "ripper/pdf/core/filter/jbig2_decode_filter.hpp"
#include "ripper/pdf/core/filter/jpx_decode_filter.hpp"
#include "ripper/pdf/core/filter/run_length_decode_filter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
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

} // namespace

TEST_CASE("unimplemented filters are recognized by the filter manager", "[filter][manager]")
{
    for (std::string_view name :
         {"RunLengthDecode", "CCITTFaxDecode", "DCTDecode", "JPXDecode", "JBIG2Decode", "Crypt"})
    {
        INFO("filter name: " << name);
        REQUIRE(filter_manager::has(name));
        REQUIRE(filter_manager::get(name) != nullptr);
    }
}

TEST_CASE("unimplemented filters throw not_implemented_exception", "[filter][not_implemented]")
{
    const auto input = b("data");

    SECTION("RunLengthDecode")
    {
        run_length_decode_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }

    SECTION("CCITTFaxDecode")
    {
        ccitt_fax_decode_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }

    SECTION("DCTDecode")
    {
        dct_decode_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }

    SECTION("JPXDecode")
    {
        jpx_decode_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }

    SECTION("JBIG2Decode")
    {
        jbig2_decode_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }

    SECTION("Crypt")
    {
        crypt_filter filter;
        REQUIRE_THROWS_AS(filter.decode(input), not_implemented_exception);
        REQUIRE_THROWS_AS(filter.encode(input), not_implemented_exception);
    }
}

TEST_CASE("filter_manager dispatches known-but-unimplemented filters",
          "[filter][manager][not_implemented]")
{
    for (std::string_view name :
         {"RunLengthDecode", "CCITTFaxDecode", "DCTDecode", "JPXDecode", "JBIG2Decode", "Crypt"})
    {
        INFO("filter name: " << name);
        dictionary_object dict;
        dict.set("Filter", object{name_object{std::string{name}}});

        REQUIRE_THROWS_AS(filter_manager::decode(dict, b("data")), not_implemented_exception);
        REQUIRE_THROWS_AS(filter_manager::encode(dict, b("data")), not_implemented_exception);
    }
}

} // namespace ripper::pdf::core