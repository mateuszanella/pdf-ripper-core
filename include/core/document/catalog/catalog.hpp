#pragma once

#include <cstdint>
#include <optional>

namespace ripper::core
{
    class catalog
    {
    public:
        catalog() = default;

        void set_pages(std::uint32_t objectNumber, std::uint16_t generation);
        [[nodiscard]] std::optional<std::uint32_t> pages_object_number() const;
        [[nodiscard]] std::optional<std::uint16_t> pages_generation() const;

    private:
        std::optional<std::uint32_t> _pagesObjectNumber;
        std::optional<std::uint16_t> _pagesGeneration;
    };
}
