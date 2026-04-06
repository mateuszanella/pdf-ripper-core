#include <print>
#include <filesystem>

#include "core/document.hpp"
#include "core/parser/parser.hpp"

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
            std::println("Failed to read PDF header. Error code: {}", static_cast<int>(header.error()));
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
            std::println("\nFailed to parse cross-reference table. Error code: {}",
                         static_cast<int>(xrefTable.error()));
        }
    }

    void check_trailer(const ripper::core::document &document)
    {
        const auto trailer = document.trailer();
        if (!trailer)
        {
            std::println("\nFailed to parse trailer. Error code: {}",
                         static_cast<int>(trailer.error()));
            return;
        }

        auto id = trailer->id();
        if (id)        {
            std::println("\nDocument ID:");

            std::println("  Original: {}", id->original());

            if (id->current()) {
                std::println("  Current: {}", *id->current());
            }
        }

        std::println("\nTrailer parsed successfully.");
    }

    void check_catalog(const ripper::core::document &document)
    {
        const auto catalog = document.catalog();
        if (!catalog)
        {
            std::println("\nFailed to parse catalog. Error code: {}",
                         static_cast<int>(catalog.error()));
            return;
        }

        std::println("\nCatalog parsed successfully.");
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = (argc > 1)
        ? std::filesystem::path{argv[1]}
        : std::filesystem::current_path() / "../example/test.pdf";

    auto document = ripper::core::document::open(path);

    if (!check_file_open(document.reader()))
    {
        return 1;
    }

    check_header(document);
    check_cross_reference_table(document);
    check_trailer(document);
    check_catalog(document);

    return 0;
}
