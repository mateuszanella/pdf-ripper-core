#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document_save_strategy/linearize_document_save_strategy.hpp"
#include "ripper/pdf/core/document_save_strategy/raw_document_save_strategy.hpp"
#include "test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>

namespace ripper::pdf::core
{
TEST_CASE("linearize save strategy writes and re-opens a created PDF",
          "[document][e2e][save][linearize]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_linearize_save_test.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();
    (void)pages.add_page();

    linearize_document_save_strategy strategy;
    REQUIRE_NOTHROW(strategy.save(doc));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 2);
}

TEST_CASE("raw save strategy writes and re-opens a created PDF", "[document][e2e][save][raw]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_raw_save_test.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();
    (void)pages.add_page();

    raw_document_save_strategy strategy;
    REQUIRE_NOTHROW(strategy.save(doc));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 2);
}

TEST_CASE("save with save_strategy_type enum selects built-in strategy",
          "[document][e2e][save][enum]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_enum_save_test.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();

    REQUIRE_NOTHROW(doc.save(save_strategy_type::linearize));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 1);
}

TEST_CASE("set_save_strategy injects a custom strategy", "[document][e2e][save][strategy]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_custom_strategy_test.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();
    (void)pages.add_page();
    (void)pages.add_page();

    doc.set_save_strategy(std::make_unique<linearize_document_save_strategy>());
    REQUIRE_NOTHROW(doc.save());
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 3);
}

TEST_CASE("set_save_strategy with nullptr resets to default", "[document][e2e][save][strategy]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_reset_strategy_test.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();

    doc.set_save_strategy(std::make_unique<linearize_document_save_strategy>());
    doc.set_save_strategy(nullptr);
    REQUIRE_NOTHROW(doc.save());

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 1);
}

TEST_CASE("save with enum ignores injected strategy", "[document][e2e][save][enum][strategy]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_enum_ignores_strategy.pdf"};

    auto doc = document::create(output.path());

    auto pages = doc.catalog().pages();
    (void)pages.add_page();
    (void)pages.add_page();

    doc.set_save_strategy(std::make_unique<raw_document_save_strategy>());
    REQUIRE_NOTHROW(doc.save(save_strategy_type::linearize));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 2);
}

TEST_CASE("raw save preserves existing PDF", "[document][e2e][save][raw]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_raw_edit_test.pdf"};

    const auto input_path = test_fixture::fixture_pdf_path();
    REQUIRE(std::filesystem::exists(input_path));

    auto reader = std::make_unique<ripper::io::core::file_reader>(input_path);
    auto writer = std::make_unique<ripper::io::core::file_writer>(output.path());
    document doc{std::move(reader), std::move(writer)};

    raw_document_save_strategy strategy;
    REQUIRE_NOTHROW(strategy.save(doc));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 3);
}

TEST_CASE("linearize save preserves existing PDF", "[document][e2e][save][linearize]")
{
    test_fixture::scoped_temp_file output{"pdf_ripper_core_linearize_edit_test.pdf"};

    const auto input_path = test_fixture::fixture_pdf_path();
    REQUIRE(std::filesystem::exists(input_path));

    auto reader = std::make_unique<ripper::io::core::file_reader>(input_path);
    auto writer = std::make_unique<ripper::io::core::file_writer>(output.path());
    document doc{std::move(reader), std::move(writer)};

    linearize_document_save_strategy strategy;
    REQUIRE_NOTHROW(strategy.save(doc));
    REQUIRE(std::filesystem::exists(output.path()));

    auto read_back = document::open(output.path());
    REQUIRE(read_back.catalog().pages().count() == 3);
}

TEST_CASE("raw save throws without writer", "[document][save][error]")
{
    auto doc = document::open(test_fixture::fixture_pdf_path());

    raw_document_save_strategy strategy;
    REQUIRE_THROWS_AS(strategy.save(doc), logic_exception);
}

TEST_CASE("linearize save throws without writer", "[document][save][error]")
{
    auto doc = document::open(test_fixture::fixture_pdf_path());

    linearize_document_save_strategy strategy;
    REQUIRE_THROWS_AS(strategy.save(doc), logic_exception);
}
} // namespace ripper::pdf::core
