#include "ripper/io/core/reader/memory_reader.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/parser/indirect_object_resolver.hpp"

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

struct offsets
{
    std::size_t header_begin;
    std::size_t obj1_begin;
    std::size_t obj2_begin;
    std::size_t xref_begin;
    std::size_t startxref_value;
};

offsets build_pdf(std::string& out)
{
    out.clear();

    const auto append = [&](std::string_view s) { out.append(s); };

    offsets result{};

    result.header_begin = out.size();
    append("%PDF-1.7\n");

    result.obj1_begin = out.size();
    append("1 0 obj\n<< /Type /Catalog >>\nendobj\n");

    result.obj2_begin = out.size();
    append("2 0 obj\n<< /Type /Pages /Count 0 /Kids [] >>\nendobj\n");

    result.xref_begin = out.size();
    append("xref\n");

    const auto xref_subsection = out.size();
    std::string subsection_line = "0 3\n"
                                  "0000000000 65535 f \n"
                                  "0000000009 00000 n \n"
                                  "0000000045 00000 n \n";

    result.startxref_value = result.xref_begin;

    append(subsection_line);

    append("trailer\n");
    append("<< /Size 3 /Root 1 0 R >>\n");
    append("startxref\n");
    append(std::to_string(result.startxref_value) + "\n");
    append("%%EOF\n");

    return result;
}
} // namespace

TEST_CASE("indirect_object_resolver resolves existing object", "[parser][resolver]")
{
    std::string pdf_content;
    build_pdf(pdf_content);
    auto data = make_bytes(pdf_content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    indirect_object_resolver resolver{doc};

    const auto raw = resolver.resolve(indirect_reference{1, 0});

    REQUIRE_FALSE(raw.empty());
    REQUIRE(raw.find("1 0 obj") != std::string::npos);
    REQUIRE(raw.find("endobj") != std::string::npos);
    REQUIRE(raw.find("/Catalog") != std::string::npos);
}

TEST_CASE("indirect_object_resolver resolves second object", "[parser][resolver]")
{
    std::string pdf_content;
    build_pdf(pdf_content);
    auto data = make_bytes(pdf_content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    indirect_object_resolver resolver{doc};

    const auto raw = resolver.resolve(indirect_reference{2, 0});

    REQUIRE_FALSE(raw.empty());
    REQUIRE(raw.find("2 0 obj") != std::string::npos);
    REQUIRE(raw.find("/Pages") != std::string::npos);
}

TEST_CASE("indirect_object_resolver throws on non-existent object", "[parser][resolver][corrupted]")
{
    std::string pdf_content;
    build_pdf(pdf_content);
    auto data = make_bytes(pdf_content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    indirect_object_resolver resolver{doc};

    REQUIRE_THROWS_WITH(resolver.resolve(indirect_reference{99, 0}),
                        Catch::Matchers::ContainsSubstring("not found"));
}

TEST_CASE("indirect_object_resolver throws on generation mismatch", "[parser][resolver][corrupted]")
{
    std::string pdf_content;
    build_pdf(pdf_content);
    auto data = make_bytes(pdf_content);

    document doc{std::make_unique<ripper::io::core::memory_reader>(data), nullptr};
    indirect_object_resolver resolver{doc};

    REQUIRE_THROWS_WITH(resolver.resolve(indirect_reference{1, 5}),
                        Catch::Matchers::ContainsSubstring("Generation mismatch"));
}
} // namespace ripper::pdf::core
