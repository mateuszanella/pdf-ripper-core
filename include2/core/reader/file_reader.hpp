#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>
#include <string_view>

#include "core/reader/reader.hpp"

namespace ripper::core
{
    class file_reader : public reader
    {
    public:
        explicit file_reader(const std::filesystem::path path);

        ~file_reader() override = default;

        file_reader(const file_reader &) = delete;
        file_reader &operator=(const file_reader &) = delete;

        file_reader(file_reader &&) noexcept = default;
        file_reader &operator=(file_reader &&) noexcept = default;

        [[nodiscard]] bool is_open() const noexcept override;
        [[nodiscard]] bool eof() const noexcept override;
        [[nodiscard]] std::uint64_t size() const noexcept override;
        [[nodiscard]] std::size_t tell() const noexcept override;

        [[nodiscard]] std::byte peek() override;

        [[nodiscard]] std::size_t read(std::span<std::byte> buffer) override;
        [[nodiscard]] std::size_t read_at(std::span<std::byte> buffer, const std::uint64_t offset) override;
        [[nodiscard]] std::size_t read_line(std::span<std::byte> buffer) override;

        void seek(std::uint64_t offset) override;
        void skip(std::size_t n) override;

        [[nodiscard]] std::string_view get_path() const noexcept;

    private:
        std::filesystem::path _path;
        std::string _canonicalPath;
        std::ifstream _handle;

        std::uint64_t _currentOffset{0};
    };
}
