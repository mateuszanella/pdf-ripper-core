#pragma once

#include <cstdint>

namespace Ripper::Core
{
    class Reader
    {
    public:
        virtual ~Reader() = default;

        [[nodiscard]] virtual bool IsOpen() const noexcept = 0;
        [[nodiscard]] virtual std::uint64_t Size() const noexcept = 0;
    };
}
