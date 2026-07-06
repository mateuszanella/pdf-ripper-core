#pragma once

#include <cstddef>
#include <span>
#include <vector>

namespace ripper::pdf::core
{

/// Pure byte container for PDF stream data.
///
/// This class holds the raw byte payload of a PDF stream object. It has no
/// knowledge of stream metadata (e.g. `/Filter`, `/Length`) or decoding state.
/// For decoded content access and filter management, use `object_stream`.
class stream
{
public:
    explicit stream(std::vector<std::byte> data) noexcept;

    /// Returns the stream bytes.
    [[nodiscard]] const std::vector<std::byte>& data() const noexcept;

    /// Returns mutable stream bytes.
    [[nodiscard]] std::vector<std::byte>& data() noexcept;

    /// Returns the current size of the stream bytes.
    [[nodiscard]] std::size_t size() const noexcept;

    /// Write bytes to the end of the stream.
    void write(std::span<std::byte> in);

private:
    std::vector<std::byte> data_;
};

} // namespace ripper::pdf::core
