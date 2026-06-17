#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/object/stream.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/default_object_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace ripper::pdf::core
{
namespace
{
document make_document()
{
    return document{nullptr, nullptr};
}

indirect_reference ref(uint32_t obj, uint16_t gen = 0)
{
    return {obj, gen};
}
} // namespace

TEST_CASE("default_object_parser parses integer object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(1), "1 0 obj\n42\nendobj\n");

    REQUIRE(result.identity().reference().object_number() == 1);
    REQUIRE(result.content().is_integer());
    REQUIRE(result.content().as_integer() != nullptr);
    REQUIRE(*result.content().as_integer() == 42);
}

TEST_CASE("default_object_parser parses dictionary object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(2), "2 0 obj\n<< /Type /Page /Count 3 >>\nendobj\n");

    REQUIRE(result.identity().reference().object_number() == 2);
    REQUIRE(result.content().is_dictionary());

    const auto* dict = result.content().as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* type = dict->get_name("Type");
    REQUIRE(type != nullptr);
    REQUIRE(type->value == "Page");

    const auto* count = dict->get_integer("Count");
    REQUIRE(count != nullptr);
    REQUIRE(*count == 3);
}

TEST_CASE("default_object_parser parses stream object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(3), "3 0 obj\n<< /Length 5 >>\nstream\nhello\nendstream\nendobj\n");

    REQUIRE(result.identity().reference().object_number() == 3);
    REQUIRE(result.content().is_stream());

    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);

    const auto& dict = obj_stream->dictionary();
    const auto* length = dict.get_integer("Length");
    REQUIRE(length != nullptr);
    REQUIRE(*length == 5);

    const auto& strm = obj_stream->stream();
    REQUIRE(strm.size() == 5);
    REQUIRE_FALSE(strm.data().empty());
}

TEST_CASE("default_object_parser parses array object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(4), "4 0 obj\n[1 2 3]\nendobj\n");

    REQUIRE(result.identity().reference().object_number() == 4);
    REQUIRE(result.content().is_array());

    const auto* arr = result.content().as_array();
    REQUIRE(arr != nullptr);
    REQUIRE(arr->size() == 3);
    REQUIRE((*arr)[0].is_integer());
    REQUIRE(*((*arr)[0].as_integer()) == 1);
    REQUIRE(*((*arr)[2].as_integer()) == 3);
}

TEST_CASE("default_object_parser parses string object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(5), "5 0 obj\n(Hello World)\nendobj\n");

    REQUIRE(result.content().is_string());
    REQUIRE(*result.content().as_string() == "Hello World");
}

TEST_CASE("default_object_parser parses boolean object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(6), "6 0 obj\ntrue\nendobj\n");

    REQUIRE(result.content().is_bool());
    REQUIRE(*result.content().as_bool() == true);
}

TEST_CASE("default_object_parser parses null object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(7), "7 0 obj\nnull\nendobj\n");

    REQUIRE(result.content().is_null());
}

TEST_CASE("default_object_parser parses indirect reference in content", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(8), "8 0 obj\n<< /Parent 1 0 R >>\nendobj\n");

    REQUIRE(result.content().is_dictionary());
    const auto* dict = result.content().as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* parent = dict->get_indirect_reference("Parent");
    REQUIRE(parent != nullptr);
    REQUIRE(parent->object_number() == 1);
    REQUIRE(parent->generation() == 0);
}

TEST_CASE("default_object_parser parses nested dictionary", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(9), "9 0 obj\n<< /Outer << /Inner 42 >> >>\nendobj\n");

    REQUIRE(result.content().is_dictionary());
    const auto* dict = result.content().as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* outer = dict->get_dictionary("Outer");
    REQUIRE(outer != nullptr);
    const auto* inner = outer->get_integer("Inner");
    REQUIRE(inner != nullptr);
    REQUIRE(*inner == 42);
}

TEST_CASE("default_object_parser skips embedded endstream text", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(10), "10 0 obj\n<< /Length 15 >>\nstream\nhello endstream\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);

    const auto& strm = obj_stream->stream();
    REQUIRE(strm.size() == 15);
}

TEST_CASE("default_object_parser parses stream with CRLF line ending", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(11), "11 0 obj\n<< /Length 4 >>\nstream\r\ntest\r\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 4);
}
} // namespace ripper::pdf::core
