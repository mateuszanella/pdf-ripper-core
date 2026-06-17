#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ripper::pdf::core
{
/// Represents the byte payload of a PDF stream object.
class stream
{
public:
    explicit stream(std::vector<std::byte> data) noexcept;

    /// Returns the decoded stream bytes.
    [[nodiscard]] const std::vector<std::byte>& data() const noexcept;

    /// Returns mutable decoded stream bytes.
    [[nodiscard]] std::vector<std::byte>& data() noexcept;

    /// Returns the current size of the decoded stream bytes.
    [[nodiscard]] std::size_t size() const noexcept;

    /// Write bytes to the end of the stream.
    void write(std::span<std::byte> in);

private:
    std::vector<std::byte> data_;
};
} // namespace ripper::pdf::core
