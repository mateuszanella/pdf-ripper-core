#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/helpers/indirect_object.hpp"
#include "ripper/pdf/core/document/object/helpers/stream.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/serializer/indirect_object/default_indirect_object_serializer.hpp"
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

document make_document()
{
    return document{nullptr, nullptr};
}
} // namespace

TEST_CASE("default_indirect_object_serializer serializes integer indirect object",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    auto obj =
        indirect_object{object_identity{&doc, {1, 0}}, object{static_cast<std::int64_t>(42)}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "1 0 obj\n42\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer serializes string indirect object",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    auto obj = indirect_object{object_identity{&doc, {5, 0}}, object{string_object{"Hello"}}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "5 0 obj\n(Hello)\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer serializes dictionary_object indirect object",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    dictionary_object dict;
    dict.set("Type", object{name_object{"Catalog"}});

    auto obj = indirect_object{object_identity{&doc, {10, 0}}, object{std::move(dict)}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "10 0 obj\n<<\n/Type /Catalog\n>>\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer serializes boolean indirect object",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    auto obj = indirect_object{object_identity{&doc, {3, 0}}, object{boolean_object{false}}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "3 0 obj\nfalse\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer serializes stream indirect object",
          "[serializer][indirect_object][stream]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    dictionary_object dict;
    dict.set("Length", object{static_cast<std::int64_t>(5)});

    std::vector<std::byte> data = {std::byte{'h'}, std::byte{'e'}, std::byte{'l'}, std::byte{'l'},
                                   std::byte{'o'}};
    stream_object obj_stream{std::move(dict), stream{std::move(data)}};

    auto obj = indirect_object{object_identity{&doc, {2, 0}}, object{std::move(obj_stream)}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) ==
            "2 0 obj\n<<\n/Length 5\n>>\nstream\nhello\nendstream\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer serializes array indirect object",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};

    array_object arr;
    arr.push_back(object{static_cast<std::int64_t>(1)});
    arr.push_back(object{static_cast<std::int64_t>(2)});
    arr.push_back(object{static_cast<std::int64_t>(3)});

    auto obj = indirect_object{object_identity{&doc, {4, 0}}, object{std::move(arr)}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "4 0 obj\n[1 2 3]\nendobj\n");
}

TEST_CASE("default_indirect_object_serializer uses custom line break character",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};
    ser.set_line_break_character('\r');

    auto obj =
        indirect_object{object_identity{&doc, {1, 0}}, object{static_cast<std::int64_t>(42)}};

    const auto result = ser.serialize(obj);

    REQUIRE(bytes_to_string(result) == "1 0 obj\r42\rendobj\r");
}

TEST_CASE("default_indirect_object_serializer propagates object break character",
          "[serializer][indirect_object]")
{
    auto doc = make_document();
    default_object_serializer obj_ser;
    default_indirect_object_serializer ser{obj_ser};
    ser.set_object_break_character('\r');

    dictionary_object dict;
    dict.set("Type", object{name_object{"Page"}});

    auto obj = indirect_object{object_identity{&doc, {7, 0}}, object{std::move(dict)}};

    const auto result = ser.serialize(obj);

    // object_break_character affects dictionary_object formatting inside the object serializer
    // line_break_character (still \n in this test) affects the indirect object envelope
    REQUIRE(bytes_to_string(result) == "7 0 obj\n<<\r/Type /Page\r>>\nendobj\n");
}
} // namespace ripper::pdf::core
