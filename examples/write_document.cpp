#include <filesystem>
#include <print>

#include "core/document.hpp"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    const std::filesystem::path path =
        std::filesystem::current_path() / "../examples" / "test.pdf";

    std::error_code ec;
    if (std::filesystem::exists(path, ec))
    {
        if (!std::filesystem::remove(path, ec))
        {
            std::print("Failed to delete existing file '{}': {}\n", path.string(), ec.message());
            return 1;
        }
    }
    else if (ec)
    {
        std::print("Failed to check if file exists '{}': {}\n", path.string(), ec.message());
        return 1;
    }

    auto document = ripper::core::document::create(path);

    auto result = document.save();
    if (!result)
    {
        std::print("Failed to save document: {}\n", result.error().message());
        return 1;
    }

    std::print("Document saved successfully.\n");
    return 0;
}
