#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/object.hpp"

#include <cstdint>
#include <vector>

namespace ripper::pdf::core
{

/// Parses a cross-reference stream (PDF 1.5+ /Type /XRef) into a cross_reference_section.
///
/// Xref streams use a binary format instead of the traditional text-based `xref` keyword.
/// The stream dictionary contains `/W` (column widths), `/Index` (subsection ranges),
/// and the stream data is decoded using the standard filter pipeline.
///
/// @see PDF spec §7.5.8
class cross_reference_stream_parser
{
public:
    /// Parse a decoded xref stream object into a cross_reference_section.
    ///
    /// The stream must already be decoded (i.e. `object_stream::content()` called).
    /// Reads `/W`, `/Index`, `/Size` from the dictionary and interprets the binary
    /// stream data according to the xref stream format.
    ///
    /// @throws parse_exception if required keys are missing or data is malformed.
    [[nodiscard]] static cross_reference_section parse(const object_stream& stream_obj);

private:
    struct column_widths
    {
        std::uint32_t w0 = 1;
        std::uint32_t w1 = 0;
        std::uint32_t w2 = 0;
    };

    struct subsection_range
    {
        std::uint32_t first;
        std::uint32_t count;
    };

    [[nodiscard]] static column_widths parse_w(const object_stream& stream_obj);
    [[nodiscard]] static std::vector<subsection_range> parse_index(const object_stream& stream_obj,
                                                                   std::uint32_t size);
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    [[nodiscard]] static std::uint64_t read_field(std::span<const std::byte> data,
                                                  std::size_t offset, std::uint32_t width);
};

} // namespace ripper::pdf::core
