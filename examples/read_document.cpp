#include <filesystem>
#include <print>

#include "core/document.hpp"

namespace
{
    bool check_file_open(const ripper::core::reader &reader)
    {
        if (reader.is_open())
        {
            std::println("PDF file is open.");

            return true;
        }

        std::println("Failed to open PDF file.");

        return false;
    }

    void check_header(const ripper::core::document &document)
    {
        const auto header = document.header();
        if (header)
        {
            std::println("PDF Header Version: {}", header.value().version());
        }
        else
        {
            const auto &err = header.error();
            std::println("Failed to read PDF header.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());

            if (err.code() == ripper::core::error_code::missing_header)
            {
                std::println("  Suggestion: File may not be a valid PDF (missing header signature)");
            }
            else if (err.code() == ripper::core::error_code::corrupted_header)
            {
                std::println("  Suggestion: PDF header is malformed; file may be corrupted");
            }
        }
    }

    void check_cross_reference_table(const ripper::core::document &document)
    {
        const auto xrefTable = document.cross_reference_table();
        if (xrefTable)
        {
            std::println("\nCross-Reference Table parsed successfully.");
            std::println("Found {} entries", xrefTable.value().size());
        }
        else
        {
            const auto &err = xrefTable.error();
            std::println("\nFailed to parse cross-reference table.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());

            if (err.code() == ripper::core::error_code::missing_xref_table)
            {
                std::println("  Suggestion: Document structure is missing xref section");
            }
            else if (err.code() == ripper::core::error_code::corrupted_xref_table)
            {
                std::println("  Suggestion: XRef table is malformed; structural integrity compromised");
            }
        }
    }

    void check_trailer(const ripper::core::document &document)
    {
        const auto trailer = document.trailer();
        if (!trailer)
        {
            const auto &err = trailer.error();
            std::println("\nFailed to parse trailer.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        auto id = trailer->id();
        if (id)
        {
            std::println("\nDocument ID:");
            std::println("  Original: {}", id->original());

            if (id->current())
            {
                std::println("  Current: {}", *id->current());
            }
        }
        else
        {
            std::println("\nDocument ID not available: {}", id.error().detailed_message());
        }

        std::println("\nTrailer parsed successfully.");
    }

    void check_catalog(const ripper::core::document &document)
    {
        const auto catalog = document.catalog();
        if (!catalog)
        {
            const auto &err = catalog.error();
            std::println("\nFailed to parse catalog.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        std::println("\nCatalog parsed successfully.");
    }

    void check_pages(const ripper::core::document &document)
    {
        const auto pages = document.catalog()->pages();
        if (!pages)
        {
            const auto &err = pages.error();
            std::println("\nFailed to parse pages.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        const auto pageCount = pages->count();
        if (!pageCount)
        {
            const auto &err = pageCount.error();
            std::println("\nFailed to get page count.");
            std::println("  Error Code: {}", static_cast<ripper::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        std::println("\nPages parsed successfully. Page count: {}", pageCount.value());
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = (argc > 1)
        ? std::filesystem::current_path() / std::filesystem::path{argv[1]}
        : std::filesystem::current_path() / "../example/test.pdf";

    auto document = ripper::core::document::open(path);

    if (!check_file_open(document.reader()->get()))
    {
        return 1;
    }

    check_header(document);
    check_cross_reference_table(document);
    check_trailer(document);
    check_catalog(document);
    check_pages(document);

    return 0;
}
