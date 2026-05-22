#include "core/document.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/file_reader.hpp"
#include "core/reader/reader.hpp"
#include "core/serializer/serializer.hpp"
#include "core/writer/file_writer.hpp"
#include "core/writer/writer.hpp"

namespace ripper::pdf::core
{
    document::document(std::unique_ptr<class reader> reader, std::unique_ptr<class writer> writer)
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
        return document{std::make_unique<ripper::pdf::core::file_reader>(path), nullptr};
    }

    document document::create(const std::filesystem::path &path)
    {
        return document{nullptr, std::make_unique<ripper::pdf::core::file_writer>(path)};
    }

    bool document::save()
    {
        if (!has_writer())
            throw logic_exception{"No writer backend available"};

        if (!has_serializer())
            throw logic_exception{"No serializer available"};

        auto &w = *writer();
        auto &s = *serializer();

        auto serialized_header = s.serialize_header(this->header());

        (void)w.write(serialized_header);

        // TODO: In reality, we should not actually use the whole xref, but use the
        // history instead. We should also resolve objects through the history.
        for (auto [number, entry_ptr] : cross_reference_table().entries())
        {
            auto &entry = *entry_ptr;
            if (!entry.in_use())
                continue;

            // Unresolved entries backed by a reader will be resolved now.
            if (!entry.is_resolved() && !entry.is_new())
                (void)resolve_object(entry.reference());

            // Reserved but never committed: skip.
            if (!entry.is_resolved())
                continue;

            // Set the offset for this entry before writing, so that the xref can be correctly generated later.
            const auto offset = static_cast<std::uint64_t>(w.tell());
            entry.set_offset(offset);

            auto data = s.serialize_indirect_object(*entry.indirect_object());

            (void)w.write(data);
        }

        // TODO: write xref
        // TODO: write trailer

        constexpr std::string_view eof_marker = "%%EOF\n";
        (void)w.write(std::as_bytes(std::span{eof_marker.data(), eof_marker.size()}));

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

    reader *document::reader() const
    {
        return reader_.get();
    }

    parser *document::parser() const
    {
        return parser_.get();
    }

    writer *document::writer() const
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

    trailer &document::trailer()
    {
        return structure().trailer();
    }

    catalog document::catalog()
    {
        auto root_ref = trailer().root();

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
