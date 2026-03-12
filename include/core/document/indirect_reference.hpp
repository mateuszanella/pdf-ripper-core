#pragma once
#include <cstdint>
#include <functional>

namespace ripper::core
{
    class indirect_reference
    {
    public:
        constexpr indirect_reference() noexcept
            : object_number_{0}, generation_{0}
        {
        }

        constexpr indirect_reference(std::uint32_t object_number, std::uint16_t generation) noexcept
            : object_number_{object_number}, generation_{generation}
        {
        }

        [[nodiscard]] constexpr std::uint32_t object_number() const noexcept
        {
            return object_number_;
        }

        [[nodiscard]] constexpr std::uint16_t generation() const noexcept
        {
            return generation_;
        }

        constexpr bool operator==(const indirect_reference &other) const noexcept = default;
        constexpr auto operator<=>(const indirect_reference &other) const noexcept = default;

    private:
        std::uint32_t object_number_;
        std::uint16_t generation_;
    };
}

template <>
struct std::hash<ripper::core::indirect_reference>
{
    std::size_t operator()(const ripper::core::indirect_reference &ref) const noexcept
    {
        auto h1 = std::hash<std::uint32_t>{}(ref.object_number());
        auto h2 = std::hash<std::uint16_t>{}(ref.generation());
        return h1 ^ (h2 << 16);
    }
};
