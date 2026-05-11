#include <filesystem>
#include <print>

#include "core/document.hpp"

namespace
{
    bool check_file_open(ripper::pdf::core::reader &reader)
    {
        if (reader.is_open())
        {
            std::println("PDF file is open.");

            return true;
        }

        std::println("Failed to open PDF file.");

        return false;
    }

    void check_header(ripper::pdf::core::document &document)
    {
        auto header = document.header();
        if (header)
        {
            std::println("PDF Header Version: {}", header->get().version());
        }
        else
        {
            const auto &err = header.error();
            std::println("Failed to read PDF header.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());

            if (err.code() == ripper::pdf::core::error_code::missing_header)
            {
                std::println("  Suggestion: File may not be a valid PDF (missing header signature)");
            }
            else if (err.code() == ripper::pdf::core::error_code::corrupted_header)
            {
                std::println("  Suggestion: PDF header is malformed; file may be corrupted");
            }
        }
    }

    void check_cross_reference_table(ripper::pdf::core::document &document)
    {
        auto xrefTable = document.cross_reference_table();
        if (xrefTable)
        {
            std::println("\nCross-Reference Table parsed successfully.");
            std::println("Found {} entries", xrefTable->get().size());
            std::println("First 5 entries:");
            size_t count = 0;
            for (const auto &[objNum, entry] : xrefTable->get().entries())
            {
                std::println("  Object {}: offset={}, gen={}, in_use={}, resolved={}",
                             objNum,
                             entry.offset().has_value()
                                 ? std::to_string(*entry.offset())
                                 : "n/a",
                             std::to_string(entry.reference().generation()),
                             entry.in_use(),
                             entry.is_resolved());

                if (++count >= 5)
                    break;
            }
        }
        else
        {
            const auto &err = xrefTable.error();
            std::println("\nFailed to parse cross-reference table.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());

            if (err.code() == ripper::pdf::core::error_code::missing_xref_table)
            {
                std::println("  Suggestion: Document structure is missing xref section");
            }
            else if (err.code() == ripper::pdf::core::error_code::corrupted_xref_table)
            {
                std::println("  Suggestion: XRef table is malformed; structural integrity compromised");
            }
        }
    }

    void check_trailer(ripper::pdf::core::document &document)
    {
        auto trailer = document.trailer();
        if (!trailer)
        {
            const auto &err = trailer.error();
            std::println("\nFailed to parse trailer.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        auto id = trailer->get().id();
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

    void check_catalog(ripper::pdf::core::document &document)
    {
        auto catalog = document.catalog();
        if (!catalog)
        {
            const auto &err = catalog.error();
            std::println("\nFailed to parse catalog.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        std::println("\nCatalog parsed successfully.");
    }

    void check_pages(ripper::pdf::core::document &document)
    {
        auto pages = document.catalog()->pages();
        if (!pages)
        {
            const auto &err = pages.error();
            std::println("\nFailed to parse pages.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        auto pageCount = pages->count();
        if (!pageCount)
        {
            const auto &err = pageCount.error();
            std::println("\nFailed to get page count.");
            std::println("  Error Code: {}", static_cast<ripper::pdf::core::error_code>(err.code()).to_string());
            std::println("  Message: {}", err.detailed_message());
            return;
        }

        auto dict = pages->dictionary();
        if (! dict)
        {
            std::println("\nPages content is not a dictionary.");
            return;
        }

        auto mediaBox = dict->get_array("MediaBox");
        if (mediaBox)
        {
            std::println("\nMediaBox found for pages:");
            std::println("  MediaBox: [");
            for (const auto &entry : *mediaBox)
            {
                if (auto *d = std::get_if<double>(&entry.variant()))
                    std::println("    {}", *d);
                else if (auto *i = std::get_if<int64_t>(&entry.variant()))
                    std::println("    {}", *i);
                else
                    std::println("    (non-numeric value)");
            }
            std::println("  ]");
        }
        else
        {
            std::println("\nMediaBox not found in pages dictionary.");
        }

        std::println("\nPages parsed successfully. Page count: {}", pageCount.value());
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = (argc > 1)
                                           ? std::filesystem::current_path() / std::filesystem::path{argv[1]}
                                           : std::filesystem::current_path() / "../example/test.pdf";

    auto document = ripper::pdf::core::document::open(path);

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
