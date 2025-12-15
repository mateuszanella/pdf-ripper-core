#include <print>
#include <filesystem>

#include "Core/Reader/FileReader.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/pdfa-1a.pdf";

    Ripper::Core::FileReader reader{path};

    if (reader.IsOpen())
    {
        std::println("PDF file is open.");
    }

    std::array<std::byte, 4096> buffer;

    std::size_t bytesRead = 0;

    while ((bytesRead = reader.Read(buffer)))
    {
        std::println("Read {} bytes.", bytesRead);
    }

    return 0;
}
