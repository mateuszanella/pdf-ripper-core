#include <filesystem>
#include <print>

#include "core/document.hpp"
#include "core/exceptions/exception.hpp"

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
        std::println("PDF Header Version: {}", document.header().version());
    }

    void check_cross_reference_table(ripper::pdf::core::document &document)
    {
        auto &xrefTable = document.cross_reference_table();
        std::println("\nCross-Reference Table parsed successfully.");
        std::println("Found {} entries", xrefTable.size());
        std::println("First 5 entries:");
        size_t count = 0;
        for (const auto &[objNum, entry] : xrefTable.entries())
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

    void check_trailer(ripper::pdf::core::document &document)
    {
        auto &trailer = document.trailer();

        auto id = trailer.id();
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
            std::println("\nDocument ID not available.");
        }

        std::println("\nTrailer parsed successfully.");
    }

    void check_catalog(ripper::pdf::core::document &document)
    {
        (void)document.catalog();

        std::println("\nCatalog parsed successfully.");
    }

    void check_pages(ripper::pdf::core::document &document)
    {
        auto pages = document.catalog().pages();

        auto pageCount = pages.count();

        auto dict = pages.dictionary();
        if (!dict)
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

        std::println("\nPages parsed successfully. Page count: {}", pageCount);
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = (argc > 1)
                                           ? std::filesystem::current_path() / std::filesystem::path{argv[1]}
                                           : std::filesystem::current_path() / "../example/test.pdf";

    try
    {
        auto document = ripper::pdf::core::document::open(path);

        if (!document.reader() || !check_file_open(*document.reader()))
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
    catch (const ripper::pdf::core::exception &ex)
    {
        std::println("Failure: {}", ex.what());
        return 1;
    }
}
