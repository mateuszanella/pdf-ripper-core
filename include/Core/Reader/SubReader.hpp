#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief A reader that operates on a subset of another reader.
     * Useful for parsing isolated sections without affecting the parent reader's state.
     */
    class SubReader : public Reader
    {
    public:
        explicit SubReader(Reader &parent, std::size_t startOffset);

        [[nodiscard]] bool IsOpen() const noexcept override;
        [[nodiscard]] bool Eof() const noexcept override;
        [[nodiscard]] std::uint64_t Size() const noexcept override;
        [[nodiscard]] std::size_t Tell() const noexcept override;

        [[nodiscard]] std::byte Peek() override;

        [[nodiscard]] std::size_t Read(std::span<std::byte> buffer) override;
        [[nodiscard]] std::size_t ReadAt(std::span<std::byte> buffer, std::uint64_t offset) override;
        [[nodiscard]] std::size_t ReadLine(std::span<std::byte> buffer) override;

        void Seek(std::uint64_t offset) override;
        void Skip(std::size_t n) override;

    private:
        Reader &_parent;
        std::size_t _startOffset;
        std::size_t _currentOffset{0};
    };
}
