#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace ripper::core
{
    /// Abstract byte-oriented output interface.
    ///
    /// A `writer` provides sequential and random-access byte emission to a backing
    /// storage (file, buffer, network, etc.). It is the output-side counterpart
    /// of `reader` and is intended for PDF serialization workflows.
    ///
    /// Implementations encapsulate storage-specific concerns such as buffering,
    /// stream state, and resource ownership.
    class writer
    {
    public:
        virtual ~writer() = default;

        /// Return `true` if this writer is open and ready for output.
        [[nodiscard]] virtual bool is_open() const noexcept = 0;

        /// Return the current stream write position.
        [[nodiscard]] virtual std::size_t tell() const noexcept = 0;

        /// Write bytes from `buffer` at the current stream position.
        ///
        /// Returns the number of bytes written.
        /// Implementations may return fewer bytes than requested on failure.
        [[nodiscard]] virtual std::size_t write(std::span<const std::byte> buffer) = 0;

        /// Move the write position to `offset` from stream start.
        virtual void seek(std::uint64_t offset) = 0;

        /// Flush buffered output to backing storage.
        virtual void flush() = 0;

        /// Close the stream and release owned resources.
        virtual void close() = 0;
    };
}
