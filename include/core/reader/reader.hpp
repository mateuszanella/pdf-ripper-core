#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace ripper::core
{
    class reader
    {
    public:
        virtual ~reader() = default;

        [[nodiscard]] virtual bool is_open() const noexcept = 0;
        [[nodiscard]] virtual bool eof() const noexcept = 0;
        [[nodiscard]] virtual std::uint64_t size() const noexcept = 0;
        [[nodiscard]] virtual std::size_t tell() const noexcept = 0;

        [[nodiscard]] virtual std::byte peek() = 0;

        [[nodiscard]] virtual std::size_t read(std::span<std::byte> buffer) = 0;
        [[nodiscard]] virtual std::size_t read_at(std::span<std::byte> buffer, const std::uint64_t offset) = 0;
        [[nodiscard]] virtual std::size_t read_line(std::span<std::byte> buffer) = 0;

        virtual void seek(std::uint64_t offset) = 0;
        virtual void skip(std::size_t n) = 0;
    };
}
