#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/serializer/cross_reference_table/compressed_cross_reference_table_serializer.hpp"
#include "ripper/pdf/core/serializer/cross_reference_table/default_cross_reference_table_serializer.hpp"
#include "ripper/pdf/core/serializer/revision_serializer.hpp"
#include "ripper/pdf/core/serializer/trailer/default_trailer_serializer.hpp"
#include "ripper/pdf/core/serializer/object/default_object_serializer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

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

cross_reference_section make_traditional_section()
{
    cross_reference_subsection::entry_map entries;
    entries.emplace(0, cross_reference_entry{{0, 65535}, 0, false});
    entries.emplace(1, cross_reference_entry{{1, 0}, 42, true});

    std::vector<cross_reference_subsection> subsections;
    subsections.emplace_back(0, std::move(entries));
    return cross_reference_section{std::move(subsections)};
}

trailer make_trailer_with_root(std::uint32_t root_obj_number)
{
    trailer t{dictionary{}};
    t.dictionary().set("Size", object{static_cast<std::int64_t>(2)});
    t.dictionary().set("Root", object{indirect_reference{root_obj_number, 0}});
    return t;
}
} // namespace

TEST_CASE("revision_serializer emits traditional xref block and trailer", "[serializer][revision]")
{
    default_object_serializer obj_ser;
    default_cross_reference_table_serializer xref_ser;
    default_trailer_serializer trailer_ser{obj_ser};

    revision_serializer ser{xref_ser, trailer_ser};

    auto section = make_traditional_section();
    auto t = make_trailer_with_root(1);

    const auto result = ser.serialize(section, t, 100);

    const auto s = bytes_to_string(result);

    // Should start with the xref block
    REQUIRE(s.find("xref\n0 2\n") == 0);
    // Should contain the trailer block
    REQUIRE(s.find("trailer\n") != std::string::npos);
    // Should contain the /Root reference
    REQUIRE(s.find("/Root") != std::string::npos);
    // Should end with startxref, offset, %%EOF
    REQUIRE(s.find("startxref\n100\n%%EOF\n") != std::string::npos);
}

TEST_CASE("revision_serializer uses custom line break for trailer_tail", "[serializer][revision]")
{
    default_object_serializer obj_ser;
    default_cross_reference_table_serializer xref_ser;
    default_trailer_serializer trailer_ser{obj_ser};
    trailer_ser.set_line_break_character('\r');

    revision_serializer ser{xref_ser, trailer_ser};

    auto section = make_traditional_section();
    auto t = make_trailer_with_root(1);

    const auto result = ser.serialize(section, t, 200);

    const auto s = bytes_to_string(result);

    // startxref should be followed by CR (\\r) instead of LF (\\n) for the tail.
    REQUIRE(s.find("startxref\r200\r%%EOF\r") != std::string::npos);
}

TEST_CASE("revision_serializer compressed section emits startxref tail without trailer keyword",
          "[serializer][revision][compressed]")
{
    // Build a section that carries compressed xref identity.
    // The compressed serializer requires the section's xref_stream_object_number to be set
    // and an object serializer wired up. For this test we wire up a minimal pair that
    // emits the xref stream bytes and verify that the revision serializer appends only
    // the startxref/%%EOF tail when is_compressed() is true.

    default_object_serializer obj_ser;
    compressed_cross_reference_table_serializer compressed_ser;
    compressed_ser.set_object_serializer(obj_ser);
    default_trailer_serializer trailer_ser{obj_ser};

    revision_serializer ser{compressed_ser, trailer_ser};

    auto section = make_traditional_section();
    section.set_xref_stream_object_number(7);
    auto t = make_trailer_with_root(1);

    const auto result = ser.serialize(section, t, 300);
    const auto s = bytes_to_string(result);

    // Compressed output begins with "N 0 obj" (where N is the section's xref-stream
    // object number), not the "xref" keyword.
    REQUIRE(s.find("7 0 obj") != std::string::npos);
    REQUIRE(s.find("/Type /XRef") != std::string::npos);
    // Trailer dictionary entries are merged into the xref stream dictionary.
    REQUIRE(s.find("/Root") != std::string::npos);
    REQUIRE(s.find("/Size") != std::string::npos);
    // The output should NOT contain the standalone trailer keyword (the dictionary is
    // incorporated into the xref stream).
    REQUIRE(s.find("trailer\n") == std::string::npos);
    // The output ends with just startxref + offset + %%EOF (no separate trailer block).
    REQUIRE(s.find("startxref\n300\n%%EOF\n") != std::string::npos);
}

} // namespace ripper::pdf::core