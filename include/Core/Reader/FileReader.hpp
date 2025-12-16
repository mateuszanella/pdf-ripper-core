#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>
#include <string_view>

#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    class FileReader : public Reader
    {
    public:
        explicit FileReader(const std::filesystem::path path);

        ~FileReader() override = default;

        FileReader(const FileReader &) = delete;
        FileReader &operator=(const FileReader &) = delete;

        FileReader(FileReader &&) noexcept = default;
        FileReader &operator=(FileReader &&) noexcept = default;

        [[nodiscard]] bool IsOpen() const noexcept override;
        [[nodiscard]] std::uint64_t Size() const noexcept override;
        [[nodiscard]] std::size_t Tell() const noexcept override;

        [[nodiscard]] std::size_t Read(std::span<std::byte> buffer) override;
        [[nodiscard]] std::size_t ReadAt(std::span<std::byte> buffer, const std::uint64_t offset) override;
        [[nodiscard]] std::size_t ReadLine(std::span<std::byte> buffer) override;

        void Seek(std::uint64_t offset) override;

        [[nodiscard]] std::string_view GetPath() const noexcept;

    private:
        std::filesystem::path m_path;
        std::string m_canonicalPath;
        std::ifstream m_handle;

        std::uint64_t m_currentOffset{0};
    };
}
