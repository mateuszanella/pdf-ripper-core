#include "core/document.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/file_reader.hpp"
#include "core/reader/reader.hpp"
#include "core/serializer/serializer.hpp"
#include "core/writer/file_writer.hpp"
#include "core/writer/writer.hpp"

namespace ripper::core
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
        return document{std::make_unique<ripper::core::file_reader>(path), nullptr};
    }

    document document::create(const std::filesystem::path &path)
    {
        return document{nullptr, std::make_unique<ripper::core::file_writer>(path)};
    }

    std::expected<bool, error> document::save()
    {
        // Sanity checks to ensure we have the necessary components to perform a save operation.
        if (!has_writer())
        {
            return std::unexpected(error_builder::create()
                                       .with_message("No writer backend available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::writer)
                                       .build());
        }

        if (!has_serializer())
        {
            return std::unexpected(error_builder::create()
                                       .with_message("No serializer available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::serializer)
                                       .build());
        }

        auto &w = writer().value().get();
        auto &s = serializer().value().get();

        // Header serialization
        auto header = this->header();
        if (!header)
        {
            return std::unexpected(header.error());
        }

        auto serialized_header = s.serialize_header(header.value());
        if (!serialized_header)
        {
            return std::unexpected(serialized_header.error());
        }

        // TODO: check for written bytes
        (void)w.write(serialized_header.value());

        // TODO: do everything else
        // The idea should probalby be:
        // - Keep track of elements that have been created in memory, not sure if
        //   through a change_set member of the document, or trough the xref itself,
        //   by adding either a flag or by just having the offest() be optional and
        //   empty for new elements
        // - Write catalog -> pages -> page, leave content empty for now
        // - Should probably add page element now
        // - Set the correct offsets on the xref using the current write position when
        //   writing each element
        // - Serialize and write xref and trailer
        // - Finally, write the EOF marker

        // EOF serialization
        constexpr std::string_view eof_marker = "%%EOF\n";

        // TODO: check for written bytes
        (void)w.write(std::as_bytes(std::span{eof_marker.data(), eof_marker.size()}));

        // Finish the output stream
        w.close();

        return true;
    }

    bool document::has_reader() const noexcept
    {
        return static_cast<bool>(reader_);
    }

    bool document::has_parser() const noexcept
    {
        return static_cast<bool>(parser_);
    }

    bool document::has_writer() const noexcept
    {
        return static_cast<bool>(writer_);
    }

    bool document::has_serializer() const noexcept
    {
        return static_cast<bool>(serializer_);
    }

    std::expected<std::reference_wrapper<reader>, error> document::reader() const noexcept
    {
        if (!reader_)
            return std::unexpected(error_builder::create()
                                       .with_message("No reader backend available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::reader)
                                       .build());

        return std::ref(*reader_);
    }

    std::expected<std::reference_wrapper<parser>, error> document::parser() const noexcept
    {
        if (!parser_)
            return std::unexpected(error_builder::create()
                                       .with_message("No parser available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::parser)
                                       .build());

        return std::ref(*parser_);
    }

    std::expected<std::reference_wrapper<writer>, error> document::writer() const noexcept
    {
        if (!writer_)
            return std::unexpected(error_builder::create()
                                       .with_message("No writer backend available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::writer)
                                       .build());

        return std::ref(*writer_);
    }

    std::expected<std::reference_wrapper<serializer>, error> document::serializer() const noexcept
    {
        if (!serializer_)
            return std::unexpected(error_builder::create()
                                       .with_message("No serializer available")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::serializer)
                                       .build());

        return std::ref(*serializer_);
    }

    std::expected<std::reference_wrapper<header>, error> document::header() noexcept
    {
        if (header_.has_value())
            return std::ref(*header_);

        auto result = has_parser()
                          ? parse_header()
                          : create_header();

        if (!result)
            return std::unexpected(result.error());

        header_ = std::move(*result);

        return std::ref(*header_);
    }

    std::expected<std::reference_wrapper<cross_reference_table>, error> document::cross_reference_table() noexcept
    {
        auto s = structure();
        if (!s)
            return std::unexpected(s.error());

        return std::ref(s->get().xref());
    }

    std::expected<std::reference_wrapper<trailer>, error> document::trailer() noexcept
    {
        auto s = structure();
        if (!s)
            return std::unexpected(s.error());

        return std::ref(s->get().trailer());
    }

    std::expected<std::reference_wrapper<catalog>, error> document::catalog() noexcept
    {
        using catalog_t = ripper::core::catalog;

        auto trailer_result = trailer();
        if (!trailer_result)
            return std::unexpected(trailer_result.error());

        auto root_ref = trailer_result->get().root();

        // No /Root yet. First access on a new document: allocate the catalog.
        if (!root_ref)
        {
            auto result = create_catalog();
            if (!result)
                return std::unexpected(result.error());
            return std::ref(*static_cast<catalog_t *>(*result));
        }

        auto xref_result = cross_reference_table();
        if (!xref_result)
            return std::unexpected(xref_result.error());

        auto *entry = xref_result->get().find(*root_ref);
        if (!entry)
            return std::unexpected(error_builder::create()
                                       .with_message("Root object not found in cross-reference table")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::catalog)
                                       .build());

        // Already resolved (cached in the entry).
        if (entry->is_resolved())
            return std::ref(*static_cast<catalog_t *>(entry->object()));

        // Lazy-load from file.
        auto result = parse_catalog();
        if (!result)
            return std::unexpected(result.error());

        return std::ref(*static_cast<catalog_t *>(*result));
    }

    std::expected<catalog *, error> document::parse_catalog() noexcept
    {
        using catalog_t = ripper::core::catalog;

        auto trailer_result = trailer();
        if (!trailer_result)
            return std::unexpected(trailer_result.error());

        auto root_ref = trailer_result->get().root();
        if (!root_ref)
            return std::unexpected(root_ref.error());

        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        auto parsed = parser_result->get().catalog();
        if (!parsed)
            return std::unexpected(parsed.error());

        auto xref_result = cross_reference_table();
        if (!xref_result)
            return std::unexpected(xref_result.error());

        auto *entry = xref_result->get().find(*root_ref);
        if (!entry)
            return std::unexpected(error_builder::create()
                                       .with_message("Root object not found in cross-reference table")
                                       .with_code(error_code::not_found)
                                       .with_component(error_component::catalog)
                                       .build());

        auto *raw = entry->resolve(std::make_unique<catalog_t>(std::move(*parsed)));

        return static_cast<catalog_t *>(raw);
    }

    std::expected<catalog *, error> document::create_catalog() noexcept
    {
        using catalog_t = ripper::core::catalog;

        auto xref_result = cross_reference_table();
        if (!xref_result)
            return std::unexpected(xref_result.error());

        auto trailer_result = trailer();
        if (!trailer_result)
            return std::unexpected(trailer_result.error());

        auto &xref = xref_result->get();
        auto ref = xref.reserve();

        dictionary dict;
        dict.set("Type", value{name{"Catalog"}});

        auto cat = std::make_unique<catalog_t>(
            object{indirect_object{*this, ref}, std::move(dict)});

        auto *raw = xref.commit(ref, std::move(cat));
        if (!raw)
            return std::unexpected(error_builder::create()
                                       .with_message("Failed to commit catalog to cross-reference table")
                                       .with_code(error_code::internal_error)
                                       .with_component(error_component::catalog)
                                       .build());

        trailer_result->get().dictionary().set("Root", value{ref});

        return static_cast<catalog_t *>(raw);
    }

    std::expected<std::reference_wrapper<document_structure>, error> document::structure() noexcept
    {
        if (structure_.has_value())
            return std::ref(*structure_);

        auto result = has_parser()
                          ? parse_structure()
                          : create_structure();

        if (!result)
            return std::unexpected(result.error());

        structure_ = std::move(*result);

        return std::ref(*structure_);
    }

    std::expected<document_structure, error> document::parse_structure() const noexcept
    {
        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        return parser_result->get().structure();
    }

    std::expected<document_structure, error> document::create_structure() const noexcept
    {
        using xref_t = ripper::core::cross_reference_table;
        using entry_t = ripper::core::cross_reference_entry;
        using iref_t = ripper::core::indirect_reference;
        using trailer_t = ripper::core::trailer;

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

    std::expected<header, error> document::parse_header() const noexcept
    {
        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        return parser_result->get().header();
    }

    std::expected<header, error> document::create_header() const noexcept
    {
        return ripper::core::header{"1.4"};
    }
}
