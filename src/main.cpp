#include <print>
#include <filesystem>

#include "Core/PDF.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/test.pdf";

    Ripper::Core::PDF pdf{path};

    auto &reader = pdf.GetReader();

    if (reader.IsOpen())
    {
        std::println("PDF file is open.");
    }

    const auto headerResult = pdf.GetParser().ReadHeader();
    if (headerResult)
    {
        std::println("PDF Header Version: {}", *headerResult);
    }
    else
    {
        std::println("Failed to read PDF header. Error code: {}", static_cast<int>(headerResult.error()));
    }

    return 0;
}
