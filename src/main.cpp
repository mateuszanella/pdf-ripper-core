#include <print>
#include <filesystem>

#include "Core/PDF.hpp"

int main(int argc, char **argv)
{
    Ripper::Core::PDF pdf{std::filesystem::current_path() / "../example/pdfa-1a.pdf"};

    std::println("PDF path is: {}", pdf.GetPath());

    return 0;
}
