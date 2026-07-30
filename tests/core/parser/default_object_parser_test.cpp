#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/helpers/indirect_object.hpp"
#include "ripper/pdf/core/document/object/helpers/stream.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
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

    const auto result = parser.parse(doc, ref(1), "\n42\n");

    REQUIRE(result.identity().reference().object_number() == 1);
    REQUIRE(result.content().is_integer());
    REQUIRE(result.content().as_number() != nullptr);
    REQUIRE(result.content().as_number()->as_integer() == 42);
}

TEST_CASE("default_object_parser parses dictionary_object object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(2), "\n<< /Type /Page /Count 3 >>\n");

    REQUIRE(result.identity().reference().object_number() == 2);
    REQUIRE(result.content().is_dictionary());

    const auto* dict = result.content().as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* type = dict->get_name("Type");
    REQUIRE(type != nullptr);
    REQUIRE(type->value == "Page");

    const auto* count = dict->get_number("Count");
    REQUIRE(count != nullptr);
    REQUIRE(count->as_integer() == 3);
}

TEST_CASE("default_object_parser parses stream object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(3), "\n<< /Length 5 >>\nstream\nhello\nendstream\n");

    REQUIRE(result.identity().reference().object_number() == 3);
    REQUIRE(result.content().is_stream());

    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);

    const auto& dict = obj_stream->dictionary();
    const auto* length = dict.get_number("Length");
    REQUIRE(length != nullptr);
    REQUIRE(length->as_integer() == 5);

    const auto& strm = obj_stream->stream();
    REQUIRE(strm.size() == 5);
    REQUIRE_FALSE(strm.data().empty());
}

TEST_CASE("default_object_parser parses array object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(4), "\n[1 2 3]\n");

    REQUIRE(result.identity().reference().object_number() == 4);
    REQUIRE(result.content().is_array());

    const auto* arr = result.content().as_array();
    REQUIRE(arr != nullptr);
    REQUIRE(arr->size() == 3);
    REQUIRE((*arr)[0].is_integer());
    REQUIRE((*arr)[0].as_number()->as_integer() == 1);
    REQUIRE((*arr)[2].as_number()->as_integer() == 3);
}

TEST_CASE("default_object_parser parses string object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(5), "\n(Hello World)\n");

    REQUIRE(result.content().is_string());
    REQUIRE(*result.content().as_string() == "Hello World");
}

TEST_CASE("default_object_parser parses boolean object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(6), "\ntrue\n");

    REQUIRE(result.content().is_boolean());
    REQUIRE(result.content().as_boolean()->value == true);
}

TEST_CASE("default_object_parser parses null object", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(7), "\nnull\n");

    REQUIRE(result.content().is_null());
}

TEST_CASE("default_object_parser parses indirect reference in content", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result = parser.parse(doc, ref(8), "\n<< /Parent 1 0 R >>\n");

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

    const auto result = parser.parse(doc, ref(9), "\n<< /Outer << /Inner 42 >> >>\n");

    REQUIRE(result.content().is_dictionary());
    const auto* dict = result.content().as_dictionary();
    REQUIRE(dict != nullptr);

    const auto* outer = dict->get_dictionary("Outer");
    REQUIRE(outer != nullptr);
    const auto* inner = outer->get_number("Inner");
    REQUIRE(inner != nullptr);
    REQUIRE(inner->as_integer() == 42);
}

TEST_CASE("default_object_parser skips embedded endstream text", "[parser][object]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(10), "\n<< /Length 15 >>\nstream\nhello endstream\nendstream\n");

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

    const auto result =
        parser.parse(doc, ref(11), "\n<< /Length 4 >>\nstream\r\ntest\r\nendstream\n");

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

    const auto result =
        parser.parse(doc, ref(12), "\n<< /Type /XObject >>\nstream\npayload\nendstream\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 7); // "payload"
}

TEST_CASE("default_object_parser handles stream with negative Length", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(13), "\n<< /Length -1 >>\nstream\nworld\nendstream\n");

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
    const auto result = parser.parse(doc, ref(14), "\n<< /Length 3 >>\nstream\nhello\nendstream\n");

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
    const auto result =
        parser.parse(doc, ref(15), "\n<< /Length 999 >>\nstream\nxray\nendstream\n");

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

    const auto result = parser.parse(doc, ref(16), "\n<< /Length 0 >>\nstream\nendstream\n");

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
    const auto result =
        parser.parse(doc, ref(17), "\n<< /Length 0 >>\nstream\nhidden\nendstream\n");

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

    const auto result =
        parser.parse(doc, ref(18), "\n<< /Length (five) >>\nstream\nchunk\nendstream\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 5); // "chunk"
}

TEST_CASE("default_object_parser handles stream where Length is a real", "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    const auto result =
        parser.parse(doc, ref(19), "\n<< /Length 2.5 >>\nstream\ntest\nendstream\n");

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
    const auto result = parser.parse(doc, ref(20), "\n<< >>\nstream\nabc\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);
    REQUIRE(obj_stream->stream().size() == 4); // "abc\n"
}

TEST_CASE("default_object_parser handles stream with multiple endstream keywords",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Two endstream keywords — rfind uses the last one
    const auto result = parser.parse(doc, ref(21), "\n<< >>\nstream\nfirst_endstream\nendstream\n");

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
    const auto result =
        parser.parse(doc, ref(22), "\n<< /Length 10 >>\nstream\ndatablock\n  \nendstream\n");

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
    const auto result =
        parser.parse(doc, ref(23), "\n<< /Length 6 >>\nstream\rfoobar\rendstream\n");

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
    const auto result = parser.parse(doc, ref(24), "\n<< /Length 4 >>\nstream rawendstream\n");

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
    const auto result =
        parser.parse(doc, ref(25), "\n<< /Length 6 >>\nstream\n123456\nendstream\n");

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

    const auto result = parser.parse(doc, ref(26), "\n<< /Length 4 >>\nstream\nword\nendstream\n");

    REQUIRE(result.content().is_stream());
    const auto* obj_stream = result.content().as_stream();
    REQUIRE(obj_stream != nullptr);

    const auto* length = obj_stream->dictionary().get_number("Length");
    REQUIRE(length != nullptr);
    REQUIRE(length->as_integer() == 4);
    REQUIRE(obj_stream->stream().size() == 4);
}

TEST_CASE(
    "default_object_parser treats non-dictionary_object content without stream as plain object",
    "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // An integer object — not a dictionary, so stream detection is skipped
    const auto result = parser.parse(doc, ref(27), "\n42\n");

    REQUIRE(result.content().is_integer());
    REQUIRE(result.content().as_number()->as_integer() == 42);
}

TEST_CASE("default_object_parser dictionary_object without stream keyword returns plain dictionary",
          "[parser][object][stream]")
{
    auto doc = make_document();
    default_object_parser parser;

    // Dictionary but no stream keyword follows
    const auto result = parser.parse(doc, ref(28), "\n<< /Key value >>\n");

    REQUIRE(result.content().is_dictionary());
    REQUIRE_FALSE(result.content().is_stream());
}
} // namespace ripper::pdf::core
