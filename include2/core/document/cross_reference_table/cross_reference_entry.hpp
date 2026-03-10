#pragma once

#include <cstdint>

namespace ripper::core
{
    /**
     * @brief Represents a single entry in a pdf cross-reference table.
     *
     * Each entry contains the byte offset, generation number, and usage status
     * for a pdf object in the document.
     */
    class cross_reference_entry
    {
    public:
        cross_reference_entry() = default;
        cross_reference_entry(std::uint64_t offset, std::uint16_t generation, bool inUse);

        [[nodiscard]] std::uint64_t offset() const { return _offset; }
        [[nodiscard]] std::uint16_t generation() const { return _generation; }
        [[nodiscard]] bool in_use() const { return _inUse; }

    private:
        std::uint64_t _offset{};
        std::uint16_t _generation{};
        bool _inUse{};
    };
}
