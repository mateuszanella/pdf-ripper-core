#include <filesystem>
#include <print>

#include "core/document.hpp"
#include "core/exceptions/exception.hpp"

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

    try
    {
        auto document = ripper::pdf::core::document::create(path);

        (void)document.catalog().pages().add_page();
        (void)document.catalog().pages().add_page();
        (void)document.catalog().pages().add_page();

        (void)document.save();

        std::print("Document saved successfully.\n");

        return 0;
    }
    catch (const ripper::pdf::core::exception &ex)
    {
        std::print("Failed to save document: {}\n", ex.what());
        return 1;
    }
}
