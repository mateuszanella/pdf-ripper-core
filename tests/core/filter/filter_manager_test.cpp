#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/filter_manager.hpp"
#include "ripper/pdf/core/filter/flate_decode_filter.hpp"

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

TEST_CASE("filter_manager get and has", "[filter][manager]")
{
    SECTION("FlateDecode is registered by default")
    {
        REQUIRE(filter_manager::has("FlateDecode"));
        REQUIRE(filter_manager::get("FlateDecode") != nullptr);
    }

    SECTION("unknown filter returns nullptr")
    {
        REQUIRE_FALSE(filter_manager::has("NonExistentFilter"));
        REQUIRE(filter_manager::get("NonExistentFilter") == nullptr);
    }
}

TEST_CASE("filter_manager register and forget", "[filter][manager]")
{
    filter_manager::register_filter("TestRegisterForget", std::make_unique<flate_decode_filter>());
    REQUIRE(filter_manager::has("TestRegisterForget"));

    filter_manager::forget("TestRegisterForget");
    REQUIRE_FALSE(filter_manager::has("TestRegisterForget"));

    SECTION("forget on unknown filter is a no-op")
    {
        REQUIRE_NOTHROW(filter_manager::forget("NonExistentFilter"));
    }
}

TEST_CASE("filter_manager decode with no /Filter", "[filter][manager]")
{
    dictionary_object dict;
    auto input = b("hello");
    auto result = filter_manager::decode(dict, input);
    REQUIRE(result == input);
}

TEST_CASE("filter_manager encode with no /Filter", "[filter][manager]")
{
    dictionary_object dict;
    auto input = b("hello");
    auto result = filter_manager::encode(dict, input);
    REQUIRE(result == input);
}

TEST_CASE("filter_manager decode with FlateDecode", "[filter][manager][flate]")
{
    dictionary_object dict;
    dict.set("Filter", object{name_object{"FlateDecode"}});

    auto input = b("Hello, Filter Manager!");
    auto compressed = filter_manager::encode(dict, input);
    auto decompressed = filter_manager::decode(dict, compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("filter_manager decode with unknown filter", "[filter][manager][error]")
{
    dictionary_object dict;
    dict.set("Filter", object{name_object{"UnknownFilter"}});

    auto input = b("hello");
    REQUIRE_THROWS_AS(filter_manager::decode(dict, input), parse_exception);
}

TEST_CASE("filter_manager decode with filter array", "[filter][manager][flate]")
{
    array_object filters;
    filters.push_back(object{name_object{"FlateDecode"}});

    dictionary_object dict;
    dict.set("Filter", object{std::move(filters)});

    auto input = b("Array filter test");
    auto compressed = filter_manager::encode(dict, input);
    auto decompressed = filter_manager::decode(dict, compressed);
    REQUIRE(decompressed == input);
}

TEST_CASE("filter_manager register custom filter", "[filter][manager]")
{
    filter_manager::register_filter("TestCustomFilter", std::make_unique<flate_decode_filter>());

    REQUIRE(filter_manager::has("TestCustomFilter"));

    dictionary_object dict;
    dict.set("Filter", object{name_object{"TestCustomFilter"}});

    auto input = b("Custom filter test");
    auto compressed = filter_manager::encode(dict, input);
    auto decompressed = filter_manager::decode(dict, compressed);
    REQUIRE(decompressed == input);

    filter_manager::forget("TestCustomFilter");
}

} // namespace ripper::pdf::core
