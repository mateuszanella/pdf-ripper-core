#include <filesystem>
#include <print>

#include "core/document.hpp"

int main(int argc, char **argv)
{
    const std::filesystem::path path = (argc > 1)
        ? std::filesystem::current_path() / std::filesystem::path{argv[1]}
        : std::filesystem::current_path() / "../example/test.pdf";

    auto document = ripper::core::document::open(path);

    return 0;
}
