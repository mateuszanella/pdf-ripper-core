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

// ── stream edge cases: Length absent / invalid ────────────────────────────────

TEST_CASE("default_object_parser handles stream with missing Length", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(12), "12 0 obj\n<< /Type /XObject >>\nstream\npayload\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 7); // "payload"
}

TEST_CASE("default_object_parser handles stream with negative Length", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(13), "13 0 obj\n<< /Length -1 >>\nstream\nworld\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 5); // "world"
}

TEST_CASE("default_object_parser handles stream with Length too small", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Length=3 but actual content "hello" is 5 bytes before endstream
    const auto result =
        parser.parse(doc, ref(14), "14 0 obj\n<< /Length 3 >>\nstream\nhello\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 5); // falls back to endstream
}

TEST_CASE("default_object_parser handles stream with Length too large", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Length=999 but actual content is only 4 bytes
    const auto result = parser.parse(
        doc, ref(15), "15 0 obj\n<< /Length 999 >>\nstream\nxray\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 4); // falls back to endstream
}

TEST_CASE("default_object_parser handles stream with zero Length and empty content",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(16), "16 0 obj\n<< /Length 0 >>\nstream\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 0);
}

TEST_CASE("default_object_parser handles stream with zero Length but content exists",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Length=0, but there is real content — cross-validation fails, falls back to endstream
    const auto result = parser.parse(
        doc, ref(17), "17 0 obj\n<< /Length 0 >>\nstream\nhidden\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 6); // "hidden"
}

TEST_CASE("default_object_parser handles stream where Length is a string",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(18), "18 0 obj\n<< /Length (five) >>\nstream\nchunk\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 5); // "chunk"
}

TEST_CASE("default_object_parser handles stream where Length is a real", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(
        doc, ref(19), "19 0 obj\n<< /Length 2.5 >>\nstream\ntest\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 4); // "test"
}

// ── stream edge cases: boundary and malformed content ─────────────────────────

TEST_CASE("default_object_parser handles stream with no endstream keyword",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // No endstream at all — entire remainder becomes the stream
    const auto result = parser.parse(doc, ref(20), "20 0 obj\n<< >>\nstream\nabc\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 11); // "abc\nendobj\n"
}

TEST_CASE("default_object_parser handles stream with multiple endstream keywords",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Two endstream keywords — rfind uses the last one
    const auto result =
        parser.parse(doc, ref(21), "21 0 obj\n<< >>\nstream\nfirst_endstream\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 15); // "first_endstream"
}

TEST_CASE("default_object_parser handles stream with whitespace before endstream",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Extra whitespace between content and endstream — stripped correctly
    const auto result = parser.parse(
        doc, ref(22), "22 0 obj\n<< /Length 10 >>\nstream\ndatablock\n  \nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 10); // "datablock\n" — Length includes trailing \n
}

TEST_CASE("default_object_parser handles stream with CR-only line ending",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // CR-only after stream — classic Mac line ending
    const auto result = parser.parse(
        doc, ref(23), "23 0 obj\n<< /Length 6 >>\nstream\rfoobar\rendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 6); // "foobar"
}

TEST_CASE("default_object_parser handles stream with no line ending after stream keyword",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // No \r or \n after "stream" — byte after keyword becomes stream content
    const auto result =
        parser.parse(doc, ref(24), "24 0 obj\n<< /Length 4 >>\nstream rawendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 4); // "raw" — Length authoritative here
}

TEST_CASE("default_object_parser handles stream with Length matching after whitespace",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Length=6, content "123456" followed by whitespace then endstream
    const auto result = parser.parse(
        doc, ref(25), "25 0 obj\n<< /Length 6 >>\nstream\n123456\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 6);
}

TEST_CASE("default_object_parser preserves stream Length value when authoritative",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(26), "26 0 obj\n<< /Length 4 >>\nstream\nword\nendstream\nendobj\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);

    const auto* length = obj_stream->dictionary().get_integer("Length");
    REQUIRE(length != nullptr);
    REQUIRE(*length == 4);
    REQUIRE(obj_stream->stream().size() == 4);
}

TEST_CASE("default_object_parser treats non-dictionary content without stream as plain object",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // An integer object — not a dictionary, so stream detection is skipped
    const auto result = parser.parse(doc, ref(27), "27 0 obj\n42\nendobj\n");

    REQUIRE(result.content().is_integer());
    REQUIRE(*result.content().as_integer() == 42);
}

TEST_CASE("default_object_parser dictionary without stream keyword returns plain dictionary",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Dictionary but no stream keyword follows
    const auto result = parser.parse(doc, ref(28), "28 0 obj\n<< /Key value >>\nendobj\n");

    REQUIRE(result.content().is_dictionary());
    REQUIRE_FALSE(result.content().is_stream());
}
} // namespace ripper::pdf::core
