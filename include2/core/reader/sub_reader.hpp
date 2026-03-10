#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "core/reader/reader.hpp"

namespace ripper::core
{
    /**
     * @brief A reader that operates on a subset of another reader.
     * Useful for parsing isolated sections without affecting the parent reader's state.
     */
    class sub_reader : public reader
    {
    public:
        explicit sub_reader(reader &parent, std::size_t startOffset);

        [[nodiscard]] bool is_open() const noexcept override;
        [[nodiscard]] bool eof() const noexcept override;
        [[nodiscard]] std::uint64_t size() const noexcept override;
        [[nodiscard]] std::size_t tell() const noexcept override;

        [[nodiscard]] std::byte peek() override;

        [[nodiscard]] std::size_t read(std::span<std::byte> buffer) override;
        [[nodiscard]] std::size_t read_at(std::span<std::byte> buffer, std::uint64_t offset) override;
        [[nodiscard]] std::size_t read_line(std::span<std::byte> buffer) override;

        void seek(std::uint64_t offset) override;
        void skip(std::size_t n) override;

    private:
        reader &_parent;
        std::size_t _startOffset;
        std::size_t _currentOffset{0};
    };
}
