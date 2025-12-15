#pragma once

#include <fstream>
#include <filesystem>
#include <cstdint>

#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class FileReader : public Reader
    {
    public:
        explicit FileReader(const std::filesystem::path &path);

        ~FileReader() override = default;

        FileReader(const FileReader &) = delete;
        FileReader &operator=(const FileReader &) = delete;

        FileReader(FileReader &&) noexcept = default;
        FileReader &operator=(FileReader &&) noexcept = default;

        [[nodiscard]] bool IsOpen() const noexcept override;
        [[nodiscard]] std::uint64_t Size() const noexcept override;

    private:
        std::ifstream m_handle;
    };
}
