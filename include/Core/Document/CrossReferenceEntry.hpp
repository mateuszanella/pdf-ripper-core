#pragma once

#include <cstdint>

namespace Ripper::Core
{
    /**
     * @brief Represents a single entry in a PDF cross-reference table.
     *
     * Each entry contains the byte offset, generation number, and usage status
     * for a PDF object in the document.
     */
    class CrossReferenceEntry
    {
    public:
        CrossReferenceEntry() = default;
        CrossReferenceEntry(std::uint64_t offset, std::uint16_t generation, bool inUse);

        [[nodiscard]] std::uint64_t Offset() const { return _offset; }
        [[nodiscard]] std::uint16_t Generation() const { return _generation; }
        [[nodiscard]] bool InUse() const { return _inUse; }

    private:
        std::uint64_t _offset{};
        std::uint16_t _generation{};
        bool _inUse{};
    };
}
