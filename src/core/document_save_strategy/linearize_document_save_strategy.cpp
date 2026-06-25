#include "ripper/pdf/core/document_save_strategy/linearize_document_save_strategy.hpp"

#include "ripper/io/core/writer/writer.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/serializer/serializer.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace ripper::pdf::core
{

void linearize_document_save_strategy::save(document& doc)
{
    // Sanity checks to ensure we have the necessary components to perform a save operation.
    if (!doc.has_writer())
        throw logic_exception{"No writer backend available"};

    if (!doc.has_serializer())
        throw logic_exception{"No serializer available"};

    // Flatten incremental update history into a single section for a full rewrite.
    doc.cross_reference_table().squash();
    doc.trailer().squash();

    // Update the trailer size to reflect the number of objects in the xref section.
    doc.trailer().active_trailer().set_size(doc.cross_reference_table().next_object_number());

    auto& xref = doc.cross_reference_table().active_section();

    // On the linearize save mode, we must do this in two passes: first,
    // resolve every object that still lives in the file (after the xref squash),
    // then serialize every active object to the output writer.
    //
    // Entry offsets still refer to positions in the input file at this point
    // (no `set_offset` has been called yet), so the resolver can safely use
    // them to determine exact byte ranges.  Objects that are already in
    // memory (resolved or newly created) are left untouched.
    for (auto [number, entry_ptr] : xref.entries())
    {
        auto& entry = *entry_ptr;

        if (!entry.in_use())
            continue;

        if (!entry.is_resolved() && !entry.is_new())
            // We can ignore the return value here; the resolver will throw if it fails.
            doc.resolve_object(entry.reference());
    }

    // By this point every in-use entry either carries a previously-resolved
    // indirect object or has been loaded by pass 1.  Offsets are now set to
    // positions in the output file; the resolver is never invoked here, so
    // no input/output offset confusion can occur.
    auto& w = *doc.writer();
    auto& s = *doc.serializer();

    auto serialized_header = s.serialize_header(doc.header());
    (void)w.write(serialized_header);

    for (auto [number, entry_ptr] : xref.entries())
    {
        auto& entry = *entry_ptr;

        // Some sanity checks to ensure the entry is in use and has an actual object set.
        if (!entry.in_use())
            continue;

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

    // Flush the cross-reference section and trailer to the output stream.
    (void)w.write(s.serialize_cross_reference_section(xref));
    (void)w.write(s.serialize_trailer(doc.trailer().active_trailer(), xref_start));

    w.flush();
    w.close();
}

} // namespace ripper::pdf::core
