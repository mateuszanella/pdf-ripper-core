#include "ripper/pdf/core/document.hpp"

#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/reader/reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/io/core/writer/writer.hpp"
#include "ripper/pdf/core/document/catalog/catalog.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/document_structure.hpp"
#include "ripper/pdf/core/document/header.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/parser.hpp"
#include "ripper/pdf/core/serializer/serializer.hpp"

#include <memory>
#include <span>
#include <utility>

namespace ripper::pdf::core
{
document::document(std::unique_ptr<ripper::io::core::reader> reader,
                   std::unique_ptr<ripper::io::core::writer> writer)
    : reader_(std::move(reader)), writer_(std::move(writer)), factory_(*this)
{
    if (reader_)
        parser_ = std::make_unique<class parser>(*this);

    if (writer_)
        serializer_ = std::make_unique<class serializer>(*this);
}

document document::open(const std::filesystem::path& path)
{
    return document{std::make_unique<ripper::io::core::file_reader>(path), nullptr};
}

document document::create(const std::filesystem::path& path)
{
    return document{nullptr, std::make_unique<ripper::io::core::file_writer>(path)};
}

void document::save()
{
    // Sanity checks to ensure we have the necessary components to perform a save operation.
    if (!has_writer())
        throw logic_exception{"No writer backend available"};

    if (!has_serializer())
        throw logic_exception{"No serializer available"};

    // Flatten incremental update history into a single section for a full rewrite.
    cross_reference_table().squash();
    trailer().squash();

    trailer().active_trailer().set_size(cross_reference_table().next_object_number());

    auto& xref = cross_reference_table().active_section();

    // On the full rewrite save mode, we must do this in two passes: first,
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
            static_cast<void>(resolve_object(entry.reference()));
    }

    // By this point every in-use entry either carries a previously-resolved
    // indirect object or has been loaded by pass 1.  Offsets are now set to
    // positions in the output file; the resolver is never invoked here, so
    // the input/output offset confusion cannot occur.
    auto& w = *writer();
    auto& s = *serializer();

    auto serialized_header = s.serialize_header(this->header());
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
    (void)w.write(s.serialize_trailer(trailer().active_trailer(), xref_start));

    w.close();
}

bool document::has_reader() const
{
    return static_cast<bool>(reader_);
}

bool document::has_parser() const
{
    return static_cast<bool>(parser_);
}

bool document::has_writer() const
{
    return static_cast<bool>(writer_);
}

bool document::has_serializer() const
{
    return static_cast<bool>(serializer_);
}

ripper::io::core::reader* document::reader() const
{
    return reader_.get();
}

parser* document::parser() const
{
    return parser_.get();
}

ripper::io::core::writer* document::writer() const
{
    return writer_.get();
}

serializer* document::serializer() const
{
    return serializer_.get();
}

header& document::header()
{
    if (header_.has_value())
        return *header_;

    header_ = has_parser() ? factory_.parse_header() : factory_.create_header();

    return *header_;
}

cross_reference_manager& document::cross_reference_table()
{
    return structure().xref();
}

trailer_manager& document::trailer()
{
    return structure().trailer();
}

catalog document::catalog()
{
    auto root_ref = trailer().compiled().root();

    if (!root_ref)
        return factory_.create_catalog();

    auto* entry = cross_reference_table().find(*root_ref);
    if (entry == nullptr)
        throw parse_exception{"Root indirect_object not found in cross-reference table"};

    if (entry->is_resolved())
        return ripper::pdf::core::catalog{*entry->indirect_object()};

    return factory_.parse_catalog();
}

indirect_object* document::resolve_object(indirect_reference ref)
{
    auto* entry = cross_reference_table().find(ref);
    if (entry == nullptr)
        throw parse_exception{"Object not found in cross-reference table"};

    if (entry->is_resolved())
        return entry->indirect_object();

    if (!has_parser())
        throw logic_exception{"No parser available to resolve unresolved indirect_object"};

    auto parsed = parser_->parse_object(ref);

    return entry->resolve(std::make_unique<indirect_object>(std::move(parsed)));
}

document_structure& document::structure()
{
    if (structure_.has_value())
        return *structure_;

    structure_ = has_parser() ? factory_.parse_structure() : factory_.create_structure();

    return *structure_;
}

object_factory& document::factory()
{
    return factory_;
}
} // namespace ripper::pdf::core
