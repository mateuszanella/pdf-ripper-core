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
    std::size_t bytesRead = reader.ReadLine(buffer);

    std::println("Bytes read: {}", bytesRead);
    std::println("Content: {}", std::string_view{reinterpret_cast<const char*>(buffer.data()), bytesRead});
    std::println("Current offset: {}", reader.Tell());

    return 0;
}
