#include <print>
#include <filesystem>

#include "Core/PDF.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/test.pdf";

    auto pdf = Ripper::Core::PDF::Open(path);

    const auto &reader = pdf.Reader();

    if (reader.IsOpen())
    {
        std::println("PDF file is open.");
    }
    else
    {
        std::println("Failed to open PDF file.");

        return 1;
    }

    auto parser = pdf.Parser();

    const auto header = parser.Header();
    if (header)
    {
        std::println("PDF Header Version: {}", header.value().Version());
    }
    else
    {
        std::println("Failed to read PDF header. Error code: {}", static_cast<int>(header.error()));
    }

    const auto xrefTable = parser.CrossReferenceTable();
    if (xrefTable)
    {
        std::println("\nCross-Reference Table parsed successfully.");
        std::println("Found {} entries", xrefTable.value().Size());
    }
    else
    {
        std::println("\nFailed to parse cross-reference table. Error code: {}", static_cast<int>(xrefTable.error()));
    }

    const auto trailer = parser.Trailer();
    if (trailer)
    {
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
    else
    {
        std::println("\nFailed to parse trailer. Error code: {}", static_cast<int>(trailer.error()));
    }

    auto breakpoints = parser.Breakpoints();

    std::println("\nFound {} breakpoints:", breakpoints.size());

    for (const auto &bp : breakpoints)
    {
        std::println(" - Position: {}, Type: {}", bp.Position(), bp.ToString());
    }

    return 0;
}
