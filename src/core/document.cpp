#include "core/document.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer_manager.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/parser.hpp"
#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/reader/reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/io/core/writer/writer.hpp"
#include "core/serializer/serializer.hpp"

namespace ripper::pdf::core
{
        document::document(std::unique_ptr<ripper::io::core::reader> reader,
                                             std::unique_ptr<ripper::io::core::writer> writer)
        : reader_(std::move(reader)),
          writer_(std::move(writer)),
          factory_(*this)
    {
        if (reader_)
            parser_ = std::make_unique<class parser>(*this);

        if (writer_)
            serializer_ = std::make_unique<class serializer>(*this);
    }

    document document::open(const std::filesystem::path &path)
    {
        return document{std::make_unique<ripper::io::core::file_reader>(path), nullptr};
    }

    document document::create(const std::filesystem::path &path)
    {
        return document{nullptr, std::make_unique<ripper::io::core::file_writer>(path)};
    }

    bool document::save()
    {
        // Sanity checks to ensure we have the necessary components to perform a save operation.
        if (!has_writer())
            throw logic_exception{"No writer backend available"};

        if (!has_serializer())
            throw logic_exception{"No serializer available"};

        auto &w = *writer();
        auto &s = *serializer();

        auto serialized_header = s.serialize_header(this->header());

        (void)w.write(serialized_header);

        // Here, we separate the entires that matter for this specific save operation (full
        // rewrite). The `active_entries()` view includes only entries that are currently in use,
        cross_reference_table().squash();
        trailer().squash();

        trailer().active_trailer().set_size(cross_reference_table().next_object_number());

        auto &xref = cross_reference_table().active_section();

        // Write all active indirect objects in ascending object number order.
        for (auto [number, entry_ptr] : xref.entries())
        {
            auto &entry = *entry_ptr;
            if (!entry.in_use())
                continue;

            // If the entry is already resolved, we can serialize it directly.
            // Otherwise, we need to resolve it first.
            const auto obj = (!entry.is_resolved() && !entry.is_new())
                                 ? resolve_object(entry.reference())
                                 : entry.indirect_object();

            // If the entry is new but unresolved, it means it was reserved but
            // never committed. We should not write it out. Parsing errors may
            // have occurred during construction, so we should not throw an
            // exception here, but simply skip it.
            if (!obj)
                continue;

            // Set the offset for this entry before writing, so that the xref
            // can be correctly generated later.
            entry.set_offset(static_cast<std::uint64_t>(w.tell()));

            (void)w.write(s.serialize_indirect_object(*obj));
        }

        auto xref_start = static_cast<std::uint64_t>(w.tell());

        (void)w.write(s.serialize_cross_reference_section(xref));
        (void)w.write(s.serialize_trailer(trailer().active_trailer(), xref_start));

        w.close();

        return true;
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

    ripper::io::core::reader *document::reader() const
    {
        return reader_.get();
    }

    parser *document::parser() const
    {
        return parser_.get();
    }

    ripper::io::core::writer *document::writer() const
    {
        return writer_.get();
    }

    serializer *document::serializer() const
    {
        return serializer_.get();
    }

    header &document::header()
    {
        if (header_.has_value())
            return *header_;

        header_ = has_parser()
                      ? factory_.parse_header()
                      : factory_.create_header();

        return *header_;
    }

    cross_reference_manager &document::cross_reference_table()
    {
        return structure().xref();
    }

    trailer_manager &document::trailer()
    {
        return structure().trailer();
    }

    catalog document::catalog()
    {
        auto root_ref = trailer().compiled().root();

        if (!root_ref)
            return factory_.create_catalog();

        auto *entry = cross_reference_table().find(*root_ref);
        if (!entry)
            throw parse_exception{"Root indirect_object not found in cross-reference table"};

        if (entry->is_resolved())
            return ripper::pdf::core::catalog{*entry->indirect_object()};

        return factory_.parse_catalog();
    }

    indirect_object *document::resolve_object(indirect_reference ref)
    {
        auto *entry = cross_reference_table().find(ref);
        if (!entry)
            throw parse_exception{"Object not found in cross-reference table"};

        if (entry->is_resolved())
            return entry->indirect_object();

        if (!has_parser())
            throw logic_exception{"No parser available to resolve unresolved indirect_object"};

        auto parsed = parser_->parse_object(ref);

        return entry->resolve(std::make_unique<indirect_object>(std::move(parsed)));
    }

    document_structure &document::structure()
    {
        if (structure_.has_value())
            return *structure_;

        structure_ = has_parser()
                         ? factory_.parse_structure()
                         : factory_.create_structure();

        return *structure_;
    }

    object_factory &document::factory()
    {
        return factory_;
    }
}
