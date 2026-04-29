#include "core/document/object/indirect_reference.hpp"

namespace ripper::core
{
    constexpr indirect_reference::indirect_reference() noexcept
        : object_number_{0}, generation_{0}
    {
    }

    constexpr indirect_reference::indirect_reference(std::uint32_t object_number, std::uint16_t generation) noexcept
        : object_number_{object_number}, generation_{generation}
    {
    }

    constexpr std::uint32_t indirect_reference::object_number() const noexcept
    {
        return object_number_;
    }

    constexpr std::uint16_t indirect_reference::generation() const noexcept
    {
        return generation_;
    }
}

std::size_t std::hash<ripper::core::indirect_reference>::operator()(const ripper::core::indirect_reference &ref) const noexcept
{
    auto h1 = std::hash<std::uint32_t>{}(ref.object_number());
    auto h2 = std::hash<std::uint16_t>{}(ref.generation());
    return h1 ^ (h2 << 16);
}
