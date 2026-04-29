#pragma once

#include <vector>
#include <cstddef>

/// Simple stream object
///
/// @todo Make this decent
class stream
{
    public:
        explicit stream(std::vector<std::byte> data) noexcept;

    private:
        std::vector<std::byte> data;
};
