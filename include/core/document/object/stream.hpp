#pragma once

#include <vector>
#include <cstddef>

/// Simple stream object
///
/// @todo Make this decent
namespace ripper::core
{
    class stream
    {
    public:
        explicit stream(std::vector<std::byte> data) noexcept;

    private:
        std::vector<std::byte> data_;
    };
}
