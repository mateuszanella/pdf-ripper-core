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
    };
}
