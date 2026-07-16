#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/revision_manager.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/parser/revision_history/default_revision_history_parser.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
std::vector<std::byte> make_bytes(std::string_view s)
{
    std::vector<std::byte> bytes(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        bytes[i] = std::byte{static_cast<unsigned char>(s[i])};
    }
    return bytes;
}

struct tracked_offsets
{
    std::size_t first_xref;
    std::size_t second_xref;
    std::size_t startxref;
};

std::string build_single_revision_pdf()
{
    std::string pdf;
    pdf += "%PDF-1.7\n";
    pdf += "1 0 obj\n<< /Type /Catalog >>\nendobj\n";

    const auto xref_begin = pdf.size();
    pdf += "xref\n";
    pdf += "0 2\n";
    pdf += "0000000000 65535 f \n";
    pdf += "0000000009 00000 n \n";
    pdf += "trailer\n";
    pdf += "<< /Size 2 /Root 1 0 R >>\n";
    pdf += "startxref\n";
    pdf += std::to_string(xref_begin) + "\n";
    pdf += "%%EOF\n";

    return pdf;
}

std::string build_multi_revision_pdf(tracked_offsets& out)
{
    std::string pdf;

    pdf += "%PDF-1.7\n";
    pdf += "1 0 obj\n<< /Type /Catalog >>\nendobj\n";

    out.first_xref = pdf.size();
    pdf += "xref\n";
    pdf += "0 2\n";
    pdf += "0000000000 65535 f \n";
    pdf += "0000000009 00000 n \n";
    pdf += "trailer\n";
    pdf += "<< /Size 2 /Root 1 0 R >>\n";

    out.second_xref = pdf.size();
    pdf += "xref\n";
    pdf += "0 2\n";
    pdf += "0000000000 65535 f \n";
    pdf += "0000000009 00000 n \n";
    pdf += "trailer\n";
    pdf += "<< /Size 2 /Root 1 0 R /Prev " + std::to_string(out.first_xref) + " >>\n";

    out.startxref = pdf.size();
    pdf += "startxref\n";
    pdf += std::to_string(out.second_xref) + "\n";
    pdf += "%%EOF\n";

    return pdf;
}
} // namespace

TEST_CASE("revision_history_parser parses single xref/trailer pair", "[parser][structure]")
{
    const auto content = build_single_revision_pdf();
    auto data = make_bytes(content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    default_revision_history_parser parser{doc};

    const auto history = parser.parse();

    REQUIRE(history->xref().size() == 2);
    REQUIRE(history->trailer().size() == 1);

    const auto* entry1 = history->xref().find(1);
    REQUIRE(entry1 != nullptr);
    REQUIRE(entry1->in_use());

    const auto& trailer = history->trailer().active_trailer();
    const auto root = trailer.root();
    REQUIRE(root.has_value());
    REQUIRE(root->object_number() == 1);
}

TEST_CASE("revision_history_parser parses multi-revision chain via Prev", "[parser][structure]")
{
    tracked_offsets offsets{};
    const auto content = build_multi_revision_pdf(offsets);
    auto data = make_bytes(content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    default_revision_history_parser parser{doc};

    const auto history = parser.parse();

    REQUIRE(history->xref().size() == 4);
    REQUIRE(history->trailer().size() == 2);

    const auto& active_trailer = history->trailer().active_trailer();
    const auto prev = active_trailer.prev();
    REQUIRE(prev.has_value());
    REQUIRE(*prev == offsets.first_xref);
}

TEST_CASE("revision_history_parser detects circular Prev references", "[parser][structure]")
{
    std::string pdf;
    pdf += "%PDF-1.7\n";
    pdf += "1 0 obj\n<< /Type /Catalog >>\nendobj\n";

    const auto xref_offset = pdf.size();
    pdf += "xref\n";
    pdf += "0 2\n";
    pdf += "0000000000 65535 f \n";
    pdf += "0000000009 00000 n \n";
    pdf += "trailer\n";

    pdf += "<< /Size 2 /Root 1 0 R /Prev " + std::to_string(xref_offset) + " >>\n";

    pdf += "startxref\n";
    pdf += std::to_string(xref_offset) + "\n";
    pdf += "%%EOF\n";

    auto data = make_bytes(pdf);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    default_revision_history_parser parser{doc};

    const auto history = parser.parse();

    REQUIRE(history->trailer().size() == 1);
}

TEST_CASE("revision_history_parser throws on missing startxref", "[parser][structure][corrupted]")
{
    std::string pdf;
    pdf += "%PDF-1.7\n";
    pdf += "1 0 obj\n<< /Type /Catalog >>\nendobj\n";

    const auto xref_begin = pdf.size();
    pdf += "xref\n0 2\n0000000000 65535 f \n0000000009 00000 n \n";
    pdf += "trailer\n<< /Size 2 /Root 1 0 R >>\n";
    pdf += "%%EOF\n";

    auto data = make_bytes(pdf);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    default_revision_history_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(), Catch::Matchers::ContainsSubstring("Missing startxref"));
}

TEST_CASE("revision_history_parser throws on invalid startxref offset",
          "[parser][structure][corrupted]")
{
    std::string pdf;
    pdf += "%PDF-1.7\n";
    pdf += "1 0 obj\n<< /Type /Catalog >>\nendobj\n";
    pdf += "xref\n0 2\n0000000000 65535 f \n0000000009 00000 n \n";
    pdf += "trailer\n<< /Size 2 /Root 1 0 R >>\n";
    pdf += "startxref\n";
    pdf += "99999999\n";
    pdf += "%%EOF\n";

    auto data = make_bytes(pdf);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    default_revision_history_parser parser{doc};

    REQUIRE_THROWS_WITH(parser.parse(),
                        Catch::Matchers::ContainsSubstring("Unable to find complete trailer"));
}
} // namespace ripper::pdf::core
