#include "ripper/pdf/core/serializer/revision_serializer.hpp"

namespace ripper::pdf::core
{

revision_serializer::revision_serializer(cross_reference_table_serializer& xref_serializer,
                                         trailer_serializer& trailer_srl) noexcept
    : xref_serializer_{xref_serializer}, trailer_serializer_{trailer_srl}
{
}

std::vector<std::byte> revision_serializer::serialize(const revision& rev,
                                                      std::uint64_t xref_offset) const
{
    std::vector<std::byte> out;

    auto xref_bytes = xref_serializer_.serialize(rev.section(), rev.trailer());
    out.insert(out.end(), xref_bytes.begin(), xref_bytes.end());

    if (rev.section().is_compressed())
    {
        auto tail = trailer_serializer_.serialize_startxref(xref_offset);
        out.insert(out.end(), tail.begin(), tail.end());
    }
    else
    {
        auto tail = trailer_serializer_.serialize(rev.trailer(), xref_offset);
        out.insert(out.end(), tail.begin(), tail.end());
    }

    return out;
}

} // namespace ripper::pdf::core
