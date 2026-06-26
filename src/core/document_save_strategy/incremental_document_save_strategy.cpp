#include "ripper/pdf/core/document_save_strategy/incremental_document_save_strategy.hpp"

#include "ripper/io/core/reader/reader.hpp"
#include "ripper/io/core/writer/writer.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/header.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/serializer/serializer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ripper::pdf::core
{

void incremental_document_save_strategy::save(document& doc)
{
    if (!doc.has_reader())
        throw logic_exception{
            "No reader backend available — incremental save requires the original file"};

    if (!doc.has_writer())
        throw logic_exception{"No writer backend available"};

    if (!doc.has_serializer())
        throw logic_exception{"No serializer available"};

    auto& sections = doc.cross_reference_table().sections();
    auto& trailers = doc.trailer().trailers();

    if (sections.empty())
        throw logic_exception{"No cross-reference sections available"};

    auto& r = *doc.reader();
    auto& w = *doc.writer();
    auto& s = *doc.serializer();

    // Step 1: Write the original file bytes verbatim from reader to writer.
    constexpr std::size_t k_copy_buf_size = 1u << 20; // 1 MiB
    r.seek(0);

    std::array<std::byte, k_copy_buf_size> copy_buf{};

    while (true)
    {
        auto bytes_read = r.read(copy_buf);
        if (bytes_read == 0)
            break;

        (void)w.write(std::span{copy_buf.data(), bytes_read});
    }

    // Step 2: Determine the /Prev chain start (the last original section's xref offset).
    std::optional<std::uint64_t> prev_xref_start;

    // Walk sections newest-to-oldest to find the last one that was written to disk.
    for (auto it = sections.rbegin(); it != sections.rend(); ++it)
    {
        if (it->startxref_offset().has_value())
        {
            prev_xref_start = it->startxref_offset();
            break;
        }
    }

    // Step 3: Write each in-memory section (no startxref_offset set) as a new revision.
    for (std::size_t i = 0; i < sections.size(); ++i)
    {
        auto& section = sections[i];

        if (section.startxref_offset().has_value())
            continue; // A section that was already written to disk.

        // Resolve every unresolved in-use entry so its indirect_object is in memory.
        for (auto [number, entry_ptr] : section.entries())
        {
            auto& entry = *entry_ptr;

            if (!entry.in_use())
                continue;

            if (!entry.is_resolved() && !entry.is_new())
                doc.resolve_object(entry.reference());
        }

        // Serialize and write all in-use objects.
        for (auto [number, entry_ptr] : section.entries())
        {
            auto& entry = *entry_ptr;

            if (!entry.in_use())
                continue;

            auto* obj = entry.indirect_object();
            if (obj == nullptr)
                continue;

            entry.set_offset(static_cast<std::uint64_t>(w.tell()));

            (void)w.write(s.serialize_indirect_object(*obj));
        }

        auto xref_start = static_cast<std::uint64_t>(w.tell());

        (void)w.write(s.serialize_cross_reference_section(section));
        section.set_startxref_offset(xref_start);

        // Write the corresponding trailer, fixing up /Prev.
        if (i < trailers.size())
        {
            auto& t = trailers[i];

            if (prev_xref_start.has_value())
                t.dictionary().set("Prev", object{static_cast<std::int64_t>(*prev_xref_start)});
            else
                t.dictionary().remove("Prev");

            (void)w.write(s.serialize_trailer(t, xref_start));
        }

        prev_xref_start = xref_start;
    }

    w.flush();
    w.close();
}

} // namespace ripper::pdf::core
