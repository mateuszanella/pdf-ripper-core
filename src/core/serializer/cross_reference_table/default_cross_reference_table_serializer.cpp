#include "core/serializer/cross_reference_table/default_cross_reference_table_serializer.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_entry.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/util/byte.hpp"

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
            ss << std::setw(10) << std::setfill('0') << offset
               << ' '
               << std::setw(5) << std::setfill('0') << generation
               << ' '
               << (in_use ? 'n' : 'f')
               << '\r' << '\n';
            return ss.str();
        }

        void append_entry(std::vector<std::byte> &out, const cross_reference_entry &entry)
        {
            byte::append_bytes(out, format_entry(
                entry.offset().value_or(0),
                static_cast<std::uint32_t>(entry.reference().generation()),
                entry.in_use()));
        }
    }

    std::vector<std::byte> default_cross_reference_table_serializer::serialize(const cross_reference_table &xref) const
    {
        const auto &entries = xref.entries();

        std::vector<std::byte> out;

        byte::append_bytes(out, "xref\n");

        if (entries.empty())
            return out;

        // Group consecutive object numbers into subsections.
        auto it = entries.begin();
        while (it != entries.end())
        {
            const std::uint32_t subsection_first = it->first;
            const auto subsection_start = it;
            std::uint32_t expected = subsection_first;

            while (it != entries.end() && it->first == expected)
            {
                ++expected;
                ++it;
            }

            const std::uint32_t count = expected - subsection_first;
            byte::append_bytes(out, std::to_string(subsection_first) + ' ' + std::to_string(count) + '\n');

            for (auto jt = subsection_start; jt != it; ++jt)
                append_entry(out, jt->second);
        }

        return out;
    }
}
