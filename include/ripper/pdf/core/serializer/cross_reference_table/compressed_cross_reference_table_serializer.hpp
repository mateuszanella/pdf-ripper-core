#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/serializer/cross_reference_table/cross_reference_table_serializer.hpp"

#include <cstdint>
#include <vector>

namespace ripper::pdf::core
{

/// Serializes a cross-reference section as a compressed xref stream (PDF 1.5+).
///
/// Implements the `cross_reference_table_serializer` interface. Delegates
/// dictionary and stream serialization to an injected `object_serializer`,
/// following the same DI pattern as `trailer_serializer`.
///
/// The xref stream is not a real document object — it encodes the xref table
/// itself. The `N 0 obj` / `endobj` envelope is added directly without
/// constructing a synthetic `indirect_object`.
///
/// @see PDF spec §7.5.8
class compressed_cross_reference_table_serializer : public cross_reference_table_serializer
{
public:
    /// Serialize a cross-reference section as an xref stream indirect object.
    [[nodiscard]] std::vector<std::byte>
    serialize(const cross_reference_section& section) const override;

    /// Inject the object serializer used to serialize the stream dictionary.
    void set_object_serializer(class object_serializer& serializer);

private:
    struct column_widths
    {
        std::uint32_t w0 = 1;
        std::uint32_t w1 = 1;
        std::uint32_t w2 = 1;
    };

    class object_serializer* object_serializer_ = nullptr;

    [[nodiscard]] column_widths compute_widths(const cross_reference_section& section) const;
    [[nodiscard]] std::vector<std::byte> encode_entries(const cross_reference_section& section,
                                                        const column_widths& widths) const;
    static void write_field(std::vector<std::byte>& out, std::uint64_t value, std::uint32_t width);
};

} // namespace ripper::pdf::core
