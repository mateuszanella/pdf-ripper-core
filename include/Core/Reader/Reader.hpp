#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace Ripper::Core
{
    class Reader
    {
    public:
        virtual ~Reader() = default;

        [[nodiscard]] virtual bool IsOpen() const noexcept = 0;
        [[nodiscard]] virtual std::uint64_t Size() const noexcept = 0;

        [[nodiscard]] virtual std::size_t Read(std::span<std::byte> buffer) = 0;
        [[nodiscard]] virtual std::size_t ReadAt(std::span<std::byte> buffer, const std::uint64_t offset) = 0;
        [[nodiscard]] virtual std::size_t ReadLine(std::span<std::byte> buffer) = 0;

        virtual void Seek(std::uint64_t offset) = 0;
    };
}
