#include <print>
#include <filesystem>

#include "Core/Reader/FileReader.hpp"
#include "Core/Parser/Parser.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/pdfa-1a.pdf";

    Ripper::Core::FileReader reader{path};

    if (reader.IsOpen())
    {
        std::println("PDF file is open.");
    }

    Ripper::Core::Parser parser{reader};
    const auto headerResult = parser.ReadHeader();
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
