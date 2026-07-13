#include "ripper/pdf/core/serializer/revision_serializer.hpp"

namespace ripper::pdf::core
{

revision_serializer::revision_serializer(cross_reference_table_serializer& xref_serializer,
                                       trailer_serializer& trailer_srl) noexcept
    : xref_serializer_{xref_serializer}, trailer_serializer_{trailer_srl}
{
}

std::vector<std::byte>
revision_serializer::serialize(const cross_reference_section& section, const trailer& trailer,
                              std::uint64_t xref_offset) const
{
    std::vector<std::byte> out;

    // Serialize the xref block (traditional) or xref stream indirect object (compressed).
    // The compressed serializer merges trailer dictionary entries into the stream dict.
    auto xref_bytes = xref_serializer_.serialize(section, trailer);
    out.insert(out.end(), xref_bytes.begin(), xref_bytes.end());

    // Append the trailer block (traditional) or just the startxref/%%EOF tail (compressed).
    if (section.is_compressed())
    {
        auto tail = trailer_serializer_.serialize_startxref(xref_offset);
        out.insert(out.end(), tail.begin(), tail.end());
    }
    else
    {
        auto tail = trailer_serializer_.serialize(trailer, xref_offset);
        out.insert(out.end(), tail.begin(), tail.end());
    }

    return out;
}

} // namespace ripper::pdf::core