#pragma once

#include <optional>
#include <vector>
#include <cstddef>

namespace ripper::pdf::core
{
    /// Represents the byte payload of a PDF stream object.
    ///
    /// A stream may be preloaded (bytes available in memory) or deferred
    /// (metadata is known but bytes have not been loaded yet).
    class stream
    {
    public:
        explicit stream(std::vector<std::byte> data) noexcept;

        /// Construct a deferred stream with expected byte length.
        [[nodiscard]] static stream deferred(std::size_t expected_size) noexcept;

        stream(const stream &other);
        stream(stream &&) noexcept = default;

        stream &operator=(const stream &other);
        stream &operator=(stream &&) noexcept = default;

        ~stream();

        /// Returns `true` when this stream payload has not been loaded yet.
        [[nodiscard]] bool is_deferred() const noexcept;

        /// Returns whether in-memory stream bytes are currently available.
        [[nodiscard]] bool has_data() const noexcept;

        /// Returns stream bytes if loaded.
        [[nodiscard]] const std::vector<std::byte> &data() const noexcept;

        /// Returns mutable stream bytes if loaded.
        [[nodiscard]] std::vector<std::byte> &data() noexcept;

        /// Returns the loaded byte count when available.
        [[nodiscard]] std::size_t size() const noexcept;

        /// Returns the expected byte count for deferred streams.
        [[nodiscard]] std::optional<std::size_t> expected_size() const noexcept;

    private:
        std::vector<std::byte> data_;
        std::optional<std::size_t> expected_size_;
        bool deferred_;
    };
}
