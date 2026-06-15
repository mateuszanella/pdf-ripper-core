#include "ripper/pdf/core/serializer/cross_reference_table/default_cross_reference_table_serializer.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/util/byte.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
/// Format a single 20-byte cross-reference entry per PDF spec §7.5.4.
///
/// Format: `nnnnnnnnnn ggggg <n|f>\r\n`
///         (10-digit offset, space, 5-digit generation, space, type, CRLF)
std::string format_entry(std::uint64_t offset, std::uint32_t generation, bool in_use)
{
    std::ostringstream ss;
    ss << std::setw(10) << std::setfill('0') << offset << ' ' << std::setw(5) << std::setfill('0')
       << generation << ' ' << (in_use ? 'n' : 'f') << '\r' << '\n';
    return ss.str();
}

void append_entry(std::vector<std::byte>& out, const cross_reference_entry& entry)
{
    byte::append_bytes(out, format_entry(entry.offset().value_or(0),
                                         static_cast<std::uint32_t>(entry.reference().generation()),
                                         entry.in_use()));
}
} // namespace

std::vector<std::byte>
default_cross_reference_table_serializer::serialize(const cross_reference_section& section) const
{
    std::vector<std::byte> out;

    byte::append_bytes(out, "xref\n");

    for (const auto& subsection : section.subsections())
    {
        byte::append_bytes(out, std::to_string(subsection.first_object_number()) + ' ' +
                                    std::to_string(subsection.count()) + '\n');

        for (const auto& [num, entry] : subsection.entries())
            append_entry(out, entry);
    }

    return out;
}
} // namespace ripper::pdf::core
