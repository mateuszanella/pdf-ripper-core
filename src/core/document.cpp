#include "core/document.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
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
          writer_(std::move(writer))
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
                      ? parse_header()
                      : create_header();

        return *header_;
    }

    cross_reference_table &document::cross_reference_table()
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
            return create_catalog();

        auto *entry = cross_reference_table().find(*root_ref);
        if (!entry)
            throw parse_exception{"Root object not found in cross-reference table"};

        if (entry->is_resolved())
            return ripper::pdf::core::catalog{*entry->object()};

        return parse_catalog();
    }

    catalog document::parse_catalog()
    {
        auto root_ref = trailer().root();
        if (!root_ref)
            throw parse_exception{"Trailer is missing required /Root reference"};

        return ripper::pdf::core::catalog{*resolve_object(*root_ref)};
    }

    catalog document::create_catalog()
    {
        auto &xref = cross_reference_table();
        auto &trl = trailer();

        auto ref = xref.reserve();

        dictionary dict;
        dict.set("Type", value{name{"Catalog"}});

        auto obj = std::make_unique<object>(indirect_object{*this, ref}, value{std::move(dict)});

        auto *raw = xref.commit(ref, std::move(obj));
        if (!raw)
            throw logic_exception{"Failed to commit catalog to cross-reference table"};

        trl.dictionary().set("Root", value{ref});

        return ripper::pdf::core::catalog{*raw};
    }

    object *document::resolve_object(indirect_reference ref)
    {
        auto *entry = cross_reference_table().find(ref);
        if (!entry)
            throw parse_exception{"Object not found in cross-reference table"};

        if (entry->is_resolved())
            return entry->object();

        if (!has_parser())
            throw logic_exception{"No parser available to resolve unresolved object"};

        auto parsed = parser_->parse_object(ref);

        return entry->resolve(std::make_unique<object>(std::move(parsed)));
    }

    document_structure &document::structure()
    {
        if (structure_.has_value())
            return *structure_;

        structure_ = has_parser()
                         ? parse_structure()
                         : create_structure();

        return *structure_;
    }

    document_structure document::parse_structure() const
    {
        if (!parser_)
            throw logic_exception{"No parser available"};

        return parser_->structure();
    }

    document_structure document::create_structure() const
    {
        using xref_t = ripper::pdf::core::cross_reference_table;
        using entry_t = ripper::pdf::core::cross_reference_entry;
        using iref_t = ripper::pdf::core::indirect_reference;
        using trailer_t = ripper::pdf::core::trailer;

        const auto generate_initial_xref = []()
        {
            xref_t::entry_map entries;
            entries.emplace(0, entry_t{iref_t{0, 65535}, 0, false});
            return xref_t{std::move(entries)};
        };

        const auto generate_initial_trailer = []()
        {
            return trailer_t{dictionary{}};
        };

        std::vector<xref_t> xref_history;
        xref_history.push_back(generate_initial_xref());

        std::vector<trailer_t> trailer_history;
        trailer_history.push_back(generate_initial_trailer());

        return document_structure{
            std::invoke(generate_initial_xref),
            std::move(xref_history),
            std::invoke(generate_initial_trailer),
            std::move(trailer_history)};
    }

    header document::parse_header() const
    {
        if (!parser_)
            throw logic_exception{"No parser available"};

        return parser_->header();
    }

    header document::create_header() const
    {
        return ripper::pdf::core::header{"1.4"};
    }
}
