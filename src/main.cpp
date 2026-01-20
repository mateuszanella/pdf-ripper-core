#include <print>
#include <filesystem>

#include "Core/PDF.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/test.pdf";

    auto pdf = Ripper::Core::PDF::Open(path);

    const auto &reader = pdf.GetReader();

    if (reader.IsOpen())
    {
        std::println("PDF file is open.");
    }
    else
    {
        std::println("Failed to open PDF file.");

        return 1;
    }

    auto parser = pdf.GetParser();

    const auto headerResult = parser.ReadHeader();
    if (headerResult)
    {
        std::println("PDF Header Version: {}", *headerResult);
    }
    else
    {
        std::println("Failed to read PDF header. Error code: {}", static_cast<int>(headerResult.error()));
    }

    const auto xrefResult = parser.ReadCrossReferenceTable();
    if (xrefResult)
    {
        std::println("Cross-Reference Table Entries: \n{}", *xrefResult);
    }
    else
    {
        std::println("Failed to read Cross-Reference Table. Error code: {}", static_cast<int>(xrefResult.error()));
    }

    return 0;
}
