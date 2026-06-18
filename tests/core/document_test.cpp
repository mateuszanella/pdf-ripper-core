#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/pdf/core/document.hpp"
#include "test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <memory>

namespace ripper::pdf::core
{
TEST_CASE("document reads a fixture PDF end-to-end", "[document][e2e][read]")
{
    const auto fixture = test_fixture::fixture_pdf_path();

    REQUIRE(std::filesystem::exists(fixture));

    auto doc = document::open(fixture);

    REQUIRE(doc.has_reader());
    REQUIRE(doc.has_parser());
    REQUIRE_FALSE(doc.has_writer());
    REQUIRE_FALSE(doc.has_serializer());

    const auto version = std::string{doc.header().version()};
    REQUIRE_FALSE(version.empty());
    REQUIRE(version.starts_with("1."));

    auto& xref = doc.cross_reference_table();
    REQUIRE(xref.size() > 0);

    auto compiled_trailer = doc.trailer().compiled();
    REQUIRE(compiled_trailer.root().has_value());

    auto catalog = doc.catalog();
    auto pages = catalog.pages();

    REQUIRE(pages.count() > 0);

    std::uint64_t iterated_pages = 0;
    pages.each([&iterated_pages](page&) { ++iterated_pages; });

    REQUIRE(iterated_pages == pages.count());
}

TEST_CASE("document writes and re-opens a created PDF", "[document][e2e][write]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_document_test_output.pdf"};

    auto writer_doc = document::create(output.path());

    REQUIRE_FALSE(writer_doc.has_reader());
    REQUIRE_FALSE(writer_doc.has_parser());
    REQUIRE(writer_doc.has_writer());
    REQUIRE(writer_doc.has_serializer());

    auto pages = writer_doc.catalog().pages();
    (void)pages.add_page();
    (void)pages.add_page();
    (void)pages.add_page();

    REQUIRE_NOTHROW(writer_doc.save());
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back_doc = document::open(output.path());
    REQUIRE(read_back_doc.catalog().pages().count() == 3);
}

TEST_CASE("document edits an already existing PDF", "[document][e2e][write]")
{
    test_fixture::scoped_temp_file output_file{"pdf_ripper_core_document_edit_test.pdf"};

    const auto input_path = test_fixture::fixture_pdf_path();
    const auto output_path = output_file.path();

    REQUIRE(std::filesystem::exists(input_path));

    auto reader = std::make_unique<ripper::io::core::file_reader>(input_path);
    auto writer = std::make_unique<ripper::io::core::file_writer>(output_path);
    document doc{std::move(reader), std::move(writer)};

    REQUIRE(doc.has_reader());
    REQUIRE(doc.has_writer());

    auto pages = doc.catalog().pages();

    const auto initial_count = pages.count();
    REQUIRE(initial_count > 0);

    pages.delete_page(2);
    REQUIRE(pages.count() == initial_count - 1);

    REQUIRE_NOTHROW(doc.save());
    REQUIRE(std::filesystem::exists(output_path));

    auto read_back = document::open(output_path);
    REQUIRE(read_back.catalog().pages().count() == initial_count - 1);
}
} // namespace ripper::pdf::core
