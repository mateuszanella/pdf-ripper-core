#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/object/stream.hpp"
#include "ripper/pdf/core/serializer/object/default_object_serializer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace ripper::pdf::core
{
namespace
{
std::string bytes_to_string(const std::vector<std::byte>& bytes)
{
    std::string result;
    result.reserve(bytes.size());
    for (auto b : bytes)
        result += static_cast<char>(b);
    return result;
}
} // namespace

// ── null ──────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes null", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{});

    REQUIRE(bytes_to_string(result) == "null");
}

// ── boolean ───────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes true", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{true});

    REQUIRE(bytes_to_string(result) == "true");
}

TEST_CASE("default_object_serializer serializes false", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{false});

    REQUIRE(bytes_to_string(result) == "false");
}

// ── integer ───────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes positive integer", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{static_cast<std::int64_t>(42)});

    REQUIRE(bytes_to_string(result) == "42");
}

TEST_CASE("default_object_serializer serializes negative integer", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{static_cast<std::int64_t>(-1)});

    REQUIRE(bytes_to_string(result) == "-1");
}

TEST_CASE("default_object_serializer serializes zero integer", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{static_cast<std::int64_t>(0)});

    REQUIRE(bytes_to_string(result) == "0");
}

TEST_CASE("default_object_serializer serializes large integer", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{static_cast<std::int64_t>(2147483647)});

    REQUIRE(bytes_to_string(result) == "2147483647");
}

// ── real ──────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes positive real", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{3.14});

    REQUIRE(bytes_to_string(result) == "3.14");
}

TEST_CASE("default_object_serializer serializes zero real", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{0.0});

    REQUIRE(bytes_to_string(result) == "0");
}

TEST_CASE("default_object_serializer serializes negative real", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{-2.5});

    REQUIRE(bytes_to_string(result) == "-2.5");
}

// ── string ────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes plain string", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"Hello World"}});

    REQUIRE(bytes_to_string(result) == "(Hello World)");
}

TEST_CASE("default_object_serializer serializes empty string", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{""}});

    REQUIRE(bytes_to_string(result) == "()");
}

TEST_CASE("default_object_serializer escapes parentheses in strings", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"a(b)c"}});

    REQUIRE(bytes_to_string(result) == "(a\\(b\\)c)");
}

TEST_CASE("default_object_serializer escapes backslash in strings", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"a\\b"}});

    REQUIRE(bytes_to_string(result) == "(a\\\\b)");
}

TEST_CASE("default_object_serializer escapes newline in strings", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"a\nb"}});

    REQUIRE(bytes_to_string(result) == "(a\\nb)");
}

TEST_CASE("default_object_serializer escapes carriage return in strings", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"a\rb"}});

    REQUIRE(bytes_to_string(result) == "(a\\rb)");
}

TEST_CASE("default_object_serializer escapes tab in strings", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{std::string{"a\tb"}});

    REQUIRE(bytes_to_string(result) == "(a\\tb)");
}

// ── name ──────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes simple name", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{name{"Type"}});

    REQUIRE(bytes_to_string(result) == "/Type");
}

TEST_CASE("default_object_serializer serializes name with special characters",
          "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{name{"A(B)C"}});

    // Names use escape_literal_string too, so parens are escaped
    REQUIRE(bytes_to_string(result) == R"(/A\(B\)C)");
}

// ── indirect reference ────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes simple indirect reference", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{indirect_reference{1, 0}});

    REQUIRE(bytes_to_string(result) == "1 0 R");
}

TEST_CASE("default_object_serializer serializes indirect reference with large generation",
          "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{indirect_reference{65535, 999}});

    REQUIRE(bytes_to_string(result) == "65535 999 R");
}

TEST_CASE("default_object_serializer serializes null indirect reference", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{indirect_reference{}});

    REQUIRE(bytes_to_string(result) == "0 0 R");
}

// ── array ─────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes empty array", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{array{}});

    REQUIRE(bytes_to_string(result) == "[]");
}

TEST_CASE("default_object_serializer serializes array of integers", "[serializer][object]")
{
    default_object_serializer ser;

    array arr;
    arr.push_back(object{static_cast<std::int64_t>(1)});
    arr.push_back(object{static_cast<std::int64_t>(2)});
    arr.push_back(object{static_cast<std::int64_t>(3)});

    const auto result = ser.serialize(object{std::move(arr)});

    REQUIRE(bytes_to_string(result) == "[1 2 3]");
}

TEST_CASE("default_object_serializer serializes array of mixed types", "[serializer][object]")
{
    default_object_serializer ser;

    array arr;
    arr.push_back(object{static_cast<std::int64_t>(42)});
    arr.push_back(object{true});
    arr.push_back(object{name{"Test"}});
    arr.push_back(object{indirect_reference{5, 0}});

    const auto result = ser.serialize(object{std::move(arr)});

    REQUIRE(bytes_to_string(result) == "[42 true /Test 5 0 R]");
}

TEST_CASE("default_object_serializer serializes nested arrays", "[serializer][object]")
{
    default_object_serializer ser;

    array inner;
    inner.push_back(object{static_cast<std::int64_t>(1)});
    inner.push_back(object{static_cast<std::int64_t>(2)});

    array outer;
    outer.push_back(object{std::move(inner)});
    outer.push_back(object{static_cast<std::int64_t>(3)});

    const auto result = ser.serialize(object{std::move(outer)});

    REQUIRE(bytes_to_string(result) == "[[1 2] 3]");
}

TEST_CASE("default_object_serializer serializes single-element array", "[serializer][object]")
{
    default_object_serializer ser;

    array arr;
    arr.push_back(object{name{"Only"}});

    const auto result = ser.serialize(object{std::move(arr)});

    REQUIRE(bytes_to_string(result) == "[/Only]");
}

// ── dictionary ────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes empty dictionary", "[serializer][object]")
{
    default_object_serializer ser;
    const auto result = ser.serialize(object{dictionary{}});

    REQUIRE(bytes_to_string(result) == "<<\n>>");
}

TEST_CASE("default_object_serializer serializes dictionary with single entry",
          "[serializer][object]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Type", object{name{"Page"}});

    const auto result = ser.serialize(object{std::move(dict)});

    REQUIRE(bytes_to_string(result) == "<<\n/Type /Page\n>>");
}

TEST_CASE("default_object_serializer serializes dictionary with multiple entries",
          "[serializer][object]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Type", object{name{"Page"}});
    dict.set("Count", object{static_cast<std::int64_t>(42)});

    const auto result = ser.serialize(object{std::move(dict)});
    const auto s = bytes_to_string(result);

    REQUIRE(s.starts_with("<<\n"));
    REQUIRE(s.ends_with("\n>>"));
    REQUIRE(s.find("/Type /Page") != std::string::npos);
    REQUIRE(s.find("/Count 42") != std::string::npos);
}

TEST_CASE("default_object_serializer serializes nested dictionary", "[serializer][object]")
{
    default_object_serializer ser;

    dictionary inner;
    inner.set("InnerKey", object{name{"InnerValue"}});

    dictionary outer;
    outer.set("Outer", object{std::move(inner)});

    const auto result = ser.serialize(object{std::move(outer)});
    const auto s = bytes_to_string(result);

    REQUIRE(s.starts_with("<<\n"));
    REQUIRE(s.ends_with("\n>>"));
    REQUIRE(s.find("/Outer") != std::string::npos);
    REQUIRE(s.find("/InnerKey /InnerValue") != std::string::npos);
}

TEST_CASE("default_object_serializer serializes dictionary with indirect reference values",
          "[serializer][object]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Root", object{indirect_reference{1, 0}});
    dict.set("Pages", object{indirect_reference{2, 0}});

    const auto result = ser.serialize(object{std::move(dict)});
    const auto s = bytes_to_string(result);

    REQUIRE(s.find("/Root 1 0 R") != std::string::npos);
    REQUIRE(s.find("/Pages 2 0 R") != std::string::npos);
}

TEST_CASE("default_object_serializer dictionary uses object break character",
          "[serializer][object]")
{
    default_object_serializer ser;
    ser.set_object_break_character('\r');

    dictionary dict;
    dict.set("Type", object{name{"Page"}});

    const auto result = ser.serialize(object{std::move(dict)});

    REQUIRE(bytes_to_string(result) == "<<\r/Type /Page\r>>");
}

// ── stream ────────────────────────────────────────────────────────────────────

TEST_CASE("default_object_serializer serializes stream object", "[serializer][object][stream]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Length", object{static_cast<std::int64_t>(5)});

    std::vector<std::byte> data = {std::byte{'h'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'},
                                   std::byte{'o'}};
    object_stream obj_stream{std::move(dict), stream{std::move(data)}};

    const auto result = ser.serialize(object{std::move(obj_stream)});

    REQUIRE(bytes_to_string(result) == "<<\n/Length 5\n>>\nstream\nhello\nendstream");
}

TEST_CASE("default_object_serializer serializes stream with empty data",
          "[serializer][object][stream]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Length", object{static_cast<std::int64_t>(0)});

    object_stream obj_stream{std::move(dict), stream{std::vector<std::byte>{}}};

    const auto result = ser.serialize(object{std::move(obj_stream)});

    REQUIRE(bytes_to_string(result) == "<<\n/Length 0\n>>\nstream\n\nendstream");
}

TEST_CASE("default_object_serializer serializes stream with dictionary other keys",
          "[serializer][object][stream]")
{
    default_object_serializer ser;

    dictionary dict;
    dict.set("Length", object{static_cast<std::int64_t>(3)});
    dict.set("Filter", object{name{"FlateDecode"}});

    std::vector<std::byte> data = {std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    object_stream obj_stream{std::move(dict), stream{std::move(data)}};

    const auto result = ser.serialize(object{std::move(obj_stream)});
    const auto s = bytes_to_string(result);

    REQUIRE(s.find("/Filter /FlateDecode") != std::string::npos);
    REQUIRE(s.find("stream") != std::string::npos);
    REQUIRE(s.find("abc") != std::string::npos);
    REQUIRE(s.find("endstream") != std::string::npos);
}

// ── line break character propagation ──────────────────────────────────────────

TEST_CASE("default_object_serializer uses custom line break for stream",
          "[serializer][object][stream]")
{
    default_object_serializer ser;
    ser.set_line_break_character('\r');

    dictionary dict;
    dict.set("Length", object{static_cast<std::int64_t>(4)});

    std::vector<std::byte> data = {std::byte{'t'}, std::byte{'e'}, std::byte{'s'}, std::byte{'t'}};
    object_stream obj_stream{std::move(dict), stream{std::move(data)}};

    const auto result = ser.serialize(object{std::move(obj_stream)});

    REQUIRE(bytes_to_string(result) == "<<\n/Length 4\n>>\rstream\rtest\rendstream");
}
} // namespace ripper::pdf::core
