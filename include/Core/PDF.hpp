#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>

namespace Ripper::Core
{
    class PDF final
    {
    public:
        explicit PDF(std::filesystem::path path);

        PDF(const PDF&) = delete;
        PDF& operator=(const PDF&) = delete;
        PDF(PDF&&) noexcept = default;
        PDF& operator=(PDF&&) noexcept = default;

        ~PDF() = default;

        [[nodiscard]] std::string_view GetPath() const noexcept;
        [[nodiscard]] bool IsOpen() const noexcept;

    private:
        std::string   m_path;
        std::ifstream m_file;
    };
}
