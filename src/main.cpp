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

        if (trailer->Size())
        {
            std::println("Size: {}", *trailer->Size());
        }

        if (trailer->RootObjectNumber())
        {
            std::println("Root: {} {} R", *trailer->RootObjectNumber(),
                trailer->RootGeneration().value_or(0));
        }

        if (trailer->InfoObjectNumber())
        {
            std::println("Info: {} {} R", *trailer->InfoObjectNumber(),
                trailer->InfoGeneration().value_or(0));
        }

        if (trailer->Prev())
        {
            std::println("Prev: {}", *trailer->Prev());
        }
    }

    void DisplayBreakpoints(Ripper::Core::Parser &parser)
    {
        const auto &breakpoints = parser.Breakpoints();

        std::println("\nFound {} breakpoints:", breakpoints.size());

        for (const auto &bp : breakpoints)
        {
            std::println(" - Position: {}, Type: {}", bp.Position(), bp.ToString());
        }
    }
}

int main(int argc, char **argv)
{
    const std::filesystem::path path = std::filesystem::current_path() / "../example/test.pdf";

    auto pdf = Ripper::Core::PDF::Open(path);

    if (!CheckFileOpen(pdf.Reader()))
    {
        return 1;
    }

    auto parser = pdf.Parser();

    CheckHeader(parser);
    CheckCrossReferenceTable(parser);
    CheckTrailer(parser);
    DisplayBreakpoints(parser);

    return 0;
}
