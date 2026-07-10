#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"

#include <cstdint>
#include <vector>

namespace ripper::pdf::core
{

/// Serializes a cross-reference section as an xref stream (PDF 1.5+).
///
/// Xref streams use a binary format instead of the traditional text-based `xref` keyword.
/// The serializer builds the `/W`, `/Index`, and stream data, then applies FlateDecode.
///
/// @see PDF spec §7.5.8
class cross_reference_stream_serializer
{
public:
    /// Serialize a cross-reference section as an xref stream object.
    ///
    /// Returns the serialized bytes of the complete indirect object
    /// (including `N G obj` ... `endobj` wrapping).
    ///
    /// @param section The cross-reference section to serialize.
    /// @param obj_number The object number to assign to the xref stream object.
    /// @param line_break The line break character to use.
    [[nodiscard]] static std::vector<std::byte> serialize(const cross_reference_section& section,
                                                          std::uint32_t obj_number,
                                                          char line_break = '\n');

private:
    struct column_widths
    {
        std::uint32_t w0 = 1;
        std::uint32_t w1 = 1;
        std::uint32_t w2 = 1;
    };

    [[nodiscard]] static column_widths compute_widths(const cross_reference_section& section);
    [[nodiscard]] static std::vector<std::byte>
    encode_entries(const cross_reference_section& section, const column_widths& widths);
    static void write_field(std::vector<std::byte>& out, std::uint64_t value, std::uint32_t width);
};

} // namespace ripper::pdf::core
