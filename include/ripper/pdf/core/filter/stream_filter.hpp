#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ripper::pdf::core
{

class stream_filter
{
public:
    virtual ~stream_filter() = default;

    [[nodiscard]] virtual std::vector<std::byte> decode(std::span<const std::byte> input) const = 0;

    [[nodiscard]] virtual std::vector<std::byte> encode(std::span<const std::byte> input) const = 0;
};

} // namespace ripper::pdf::core
