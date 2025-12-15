#include <print>
#include <filesystem>

#include "Core/Reader/FileReader.hpp"

int main(int argc, char **argv)
{
    std::filesystem::path path = std::filesystem::current_path() / "../example/pdfa-1a.pdf";

    Ripper::Core::FileReader reader{path};

    if (reader.IsOpen()) {
        std::println("PDF file is open.");
    }

    std::println("PDF size is: {}", reader.Size());

    return 0;
}
