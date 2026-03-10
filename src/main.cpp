#include <print>
#include <filesystem>

#include "core/pdf.hpp"

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

    void check_header(ripper::core::parser &parser)
    {
        const auto header = parser.header();
        if (header)
        {
            std::println("PDF Header Version: {}", header.value().version());
        }
        else
        {
            std::println("Failed to read PDF header. Error code: {}", static_cast<int>(header.error()));
        }
    }

    void check_cross_reference_table(ripper::core::parser &parser)
    {
        const auto xrefTable = parser.cross_reference_table();
        if (xrefTable)
        {
            std::println("\nCross-Reference Table parsed successfully.");
            std::println("Found {} entries", xrefTable.value().size());

            const auto history = parser.cross_reference_table_history();
            if (history)
            {
                std::println("Found {} xref tables in document", history.value().size());
            }

            // for (const auto& [objNum, entry] : xrefTable.value().entries())
            // {
            //     std::println("Object {}: offset={}, generation={}, inUse={}",
            //                  objNum,
            //                  entry.offset(),
            //                  entry.generation(),
            //                  entry.in_use() ? "yes" : "no");
            // }
        }
        else
        {
            std::println("\nFailed to parse cross-reference table. Error code: {}",
                         static_cast<int>(xrefTable.error()));
        }
    }

    void check_trailer(ripper::core::parser &parser)
    {
        const auto trailer = parser.trailer();
        if (!trailer)
        {
            std::println("\nFailed to parse trailer. Error code: {}",
                         static_cast<int>(trailer.error()));
            return;
        }

        std::println("\nTrailer parsed successfully.");

        // size
        if (trailer.value().size())
        {
            std::println("/Size: {}", *trailer.value().size());
        }

        // Root
        if (trailer.value().root_object_number() && trailer.value().root_generation())
        {
            std::println("/Root: {} {}",
                         *trailer.value().root_object_number(),
                         *trailer.value().root_generation());
        }

        // id
        if (trailer.value().id())
        {
            const auto& [originalId, currentId] = *trailer.value().id();
            std::println("/ID[0] (original): {}", originalId);
            if (currentId)
            {
                std::println("/ID[1] (current):  {}", *currentId);
            }
        }
    }

    void check_catalog(ripper::core::parser &parser)
    {
        const auto catalog = parser.catalog();
        if (!catalog)
        {
            std::println("\nFailed to parse catalog. Error code: {}",
                         static_cast<int>(catalog.error()));
            return;
        }

        std::println("\nCatalog parsed successfully.");

        if (catalog.value().pages_object_number() && catalog.value().pages_generation())
        {
            std::println("/Pages: {} {}",
                         *catalog.value().pages_object_number(),
                         *catalog.value().pages_generation());
        }
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = std::filesystem::current_path() / "../example/test.pdf";

    auto pdf = ripper::core::pdf::open(path);

    if (!check_file_open(pdf.reader()))
    {
        return 1;
    }

    auto parser = pdf.parser();

    auto result = parser.ensure_parsed();
    if (result)
    {
        check_header(parser);
        check_cross_reference_table(parser);
        check_trailer(parser);
        check_catalog(parser);
    }

    return 0;
}
