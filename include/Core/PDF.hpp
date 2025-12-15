#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>

namespace Ripper::Core
{
    class PDF
    {
    public:
        explicit PDF(std::filesystem::path path);

        [[nodiscard]] std::string_view GetPath() const noexcept;
        [[nodiscard]] bool IsOpen() const noexcept;

    private:
        std::string   m_path;
        std::ifstream m_file;
    };
}
