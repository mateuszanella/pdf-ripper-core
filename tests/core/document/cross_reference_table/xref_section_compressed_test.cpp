#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"

#include <catch2/catch_test_macros.hpp>
#include <optional>

namespace ripper::pdf::core
{

TEST_CASE("cross_reference_section is traditional by default",
          "[cross_reference_section][compressed]")
{
    cross_reference_section section{{}};
    REQUIRE_FALSE(section.is_compressed());
    REQUIRE_FALSE(section.xref_stream_object_number().has_value());
}

TEST_CASE("cross_reference_section set_xref_stream_object_number marks as compressed",
          "[cross_reference_section][compressed]")
{
    cross_reference_section section{{}};
    section.set_xref_stream_object_number(42);
    REQUIRE(section.is_compressed());
    REQUIRE(section.xref_stream_object_number().has_value());
    REQUIRE(*section.xref_stream_object_number() == 42);
}

TEST_CASE("cross_reference_section clearing xref_stream_object_number reverts to traditional",
          "[cross_reference_section][compressed]")
{
    cross_reference_section section{{}};
    section.set_xref_stream_object_number(7);
    REQUIRE(section.is_compressed());

    section.set_xref_stream_object_number(std::nullopt);
    REQUIRE_FALSE(section.is_compressed());
    REQUIRE_FALSE(section.xref_stream_object_number().has_value());
}

TEST_CASE("cross_reference_section xref_stream_object_number survives copy",
          "[cross_reference_section][compressed]")
{
    cross_reference_section section{{}};
    section.set_xref_stream_object_number(13);

    cross_reference_section copy{section};
    REQUIRE(copy.is_compressed());
    REQUIRE(*copy.xref_stream_object_number() == 13);
}

} // namespace ripper::pdf::core