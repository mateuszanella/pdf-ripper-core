#include "core/document/object/stream.hpp"

namespace ripper::core
{
    stream::stream(std::vector<std::byte> data) noexcept
        : data_(std::move(data))
    {
    }
}
