#include <print>
#include <filesystem>

#include "Core/PDF.hpp"

namespace
{
    bool CheckFileOpen(const Ripper::Core::Reader &reader)
    {
        if (reader.IsOpen())
        {
            std::println("PDF file is open.");
            return true;
        }

        std::println("Failed to open PDF file.");
        return false;
    }

    void CheckHeader(Ripper::Core::Parser &parser)
    {
        const auto header = parser.Header();
        if (header)
        {
            std::println("PDF Header Version: {}", header.value().Version());
        }
        else
        {
            std::println("Failed to read PDF header. Error code: {}", static_cast<int>(header.error()));
        }
    }

    void CheckCrossReferenceTable(Ripper::Core::Parser &parser)
    {
        const auto xrefTable = parser.CrossReferenceTable();
        if (xrefTable)
        {
            std::println("\nCross-Reference Table parsed successfully.");
            std::println("Found {} entries", xrefTable.value().Size());

            const auto history = parser.CrossReferenceTableHistory();
            if (history)
            {
                std::println("Found {} xref tables in document", history.value().size());
            }

            // for (const auto& [objNum, entry] : xrefTable.value().Entries())
            // {
            //     std::println("Object {}: offset={}, generation={}, inUse={}",
            //                  objNum,
            //                  entry.Offset(),
            //                  entry.Generation(),
            //                  entry.InUse() ? "yes" : "no");
            // }
        }
        else
        {
            std::println("\nFailed to parse cross-reference table. Error code: {}",
                         static_cast<int>(xrefTable.error()));
        }
    }

    void CheckTrailer(Ripper::Core::Parser &parser)
    {
        const auto trailer = parser.Trailer();
        if (!trailer)
        {
            std::println("\nFailed to parse trailer. Error code: {}",
                         static_cast<int>(trailer.error()));
            return;
        }

        std::println("\nTrailer parsed successfully.");

        // Size
        if (trailer.value().Size())
        {
            std::println("/Size: {}", *trailer.value().Size());
        }

        // Root
        if (trailer.value().RootObjectNumber() && trailer.value().RootGeneration())
        {
            std::println("/Root: {} {}",
                         *trailer.value().RootObjectNumber(),
                         *trailer.value().RootGeneration());
        }

        // ID
        if (trailer.value().ID())
        {
            const auto& [originalId, currentId] = *trailer.value().ID();
            std::println("/ID[0] (original): {}", originalId);
            if (currentId)
            {
                std::println("/ID[1] (current):  {}", *currentId);
            }
        }
    }

    void CheckCatalog(Ripper::Core::Parser &parser)
    {
        const auto catalog = parser.Catalog();
        if (!catalog)
        {
            std::println("\nFailed to parse catalog. Error code: {}",
                         static_cast<int>(catalog.error()));
            return;
        }

        std::println("\nCatalog parsed successfully.");

        if (catalog.value().PagesObjectNumber() && catalog.value().PagesGeneration())
        {
            std::println("/Pages: {} {}",
                         *catalog.value().PagesObjectNumber(),
                         *catalog.value().PagesGeneration());
        }

        if (catalog.value().Lang())
        {
            std::println("/Lang: {}", *catalog.value().Lang());
        }
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = std::filesystem::current_path() / "../example/pades.pdf";

    auto pdf = Ripper::Core::PDF::Open(path);

    if (!CheckFileOpen(pdf.Reader()))
    {
        return 1;
    }

    auto parser = pdf.Parser();

    auto result = parser.EnsureParsed();
    if (result)
    {
        CheckHeader(parser);
        CheckCrossReferenceTable(parser);
        CheckTrailer(parser);
        CheckCatalog(parser);
    }

    return 0;
}
