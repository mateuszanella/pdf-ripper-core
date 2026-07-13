#include "ripper/pdf/core/document_save_strategy/raw_document_save_strategy.hpp"

#include "ripper/io/core/writer/writer.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/serializer/serializer.hpp"

#include <cstddef>
#include <cstdint>

namespace ripper::pdf::core
{

void raw_document_save_strategy::save(document& doc)
{
    // Sanity checks to ensure we have the necessary components to perform a save operation.
    if (!doc.has_writer())
        throw logic_exception{"No writer backend available"};

    if (!doc.has_serializer())
        throw logic_exception{"No serializer available"};

    auto& sections = doc.cross_reference_table().sections();
    auto& trailers = doc.trailer().trailers();

    if (sections.empty())
        throw logic_exception{"No cross-reference sections available"};

    // First pass: resolve every unresolved in-use entry across all sections
    // so that its indirect_object is available for serialization.
    //
    // Free-list entries are skipped. They carry no object data on disk and
    // their reference points to the next free entry, not to a real object.
    // Objects attached to free entries in memory are still written out in
    // the second pass via the obj == nullptr guard.
    for (auto& section : sections)
    {
        for (auto [number, entry_ptr] : section.entries())
        {
            auto& entry = *entry_ptr;

            // These should be free-list entries that carry no object data on disk.
            if (!entry.in_use())
                continue;

            if (!entry.is_resolved() && !entry.is_new())
                // We can ignore the return value here; the resolver will throw if it fails.
                doc.resolve_object(entry.reference());
        }
    }

    auto& w = *doc.writer();
    auto& s = *doc.serializer();

    // Serialize the PDF header once.
    auto serialized_header = s.serialize_header(doc.header());
    (void)w.write(serialized_header);

    // Second pass: walk each xref section (revision) in chronological order,
    // serializing every entry that carries an in-memory object, its
    // cross-reference block, and its corresponding trailer.  This preserves
    // the full multi-revision structure exactly as it exists in memory.
    //
    // An object is written unconditionally if it lives in memory, regardless
    // of the entry's in_use flag — a free-list entry that happens to hold a
    // resolved object is still serialised.  The caller owns correctness.
    for (std::size_t i = 0; i < sections.size(); ++i)
    {
        auto& section = sections[i];

        for (auto [number, entry_ptr] : section.entries())
        {
            auto& entry = *entry_ptr;

            auto* obj = entry.indirect_object();
            if (obj == nullptr)
                continue;

            // Save the current output position as the offset for this entry to ensure
            // the cross-reference table entry points to the correct location in the output.
            entry.set_offset(static_cast<std::uint64_t>(w.tell()));

            // Serialize the indirect object to the output stream.
            (void)w.write(s.serialize_indirect_object(*obj));
        }

        auto xref_start = static_cast<std::uint64_t>(w.tell());

        // Serialize the revision (xref block + trailer + startxref + %%EOF) as a
        // single unit. For compressed sections the trailer dictionary is merged
        // into the xref stream dictionary; for traditional sections the trailer
        // block is emitted as-is — the caller owns `/Prev` correctness.
        const auto& t = (i < trailers.size()) ? trailers[i] : trailer{dictionary{}};
        (void)w.write(s.serialize_revision(section, t, xref_start));
    }

    w.flush();
    w.close();
}

} // namespace ripper::pdf::core
