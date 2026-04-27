#include "core/document.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
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

    std::expected<header, error> document::header() const noexcept
    {
        if (header_.has_value())
            return header_.value();

        auto result = has_parser()
                          ? parse_header()
                          : create_header();

        if (!result)
            return std::unexpected(result.error());

        header_ = std::move(*result);

        return header_.value();
    }

    std::expected<cross_reference_table, error> document::cross_reference_table() const noexcept
    {
        if (xref_table_.has_value())
            return xref_table_.value();

        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        auto parsed = parser_result->get().structure();
        if (!parsed)
            return std::unexpected(parsed.error());

        xref_table_ = std::move(parsed->compiled_xref);
        xref_history_ = std::move(parsed->xref_history);
        trailer_ = std::move(parsed->compiled_trailer);
        trailer_history_ = std::move(parsed->trailer_history);

        return xref_table_.value();
    }

    std::expected<trailer, error> document::trailer() const noexcept
    {
        if (trailer_.has_value())
            return trailer_.value();

        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        auto parsed = parser_result->get().structure();
        if (!parsed)
            return std::unexpected(parsed.error());

        xref_table_ = std::move(parsed->compiled_xref);
        xref_history_ = std::move(parsed->xref_history);
        trailer_ = std::move(parsed->compiled_trailer);
        trailer_history_ = std::move(parsed->trailer_history);

        return trailer_.value();
    }

    std::expected<catalog, error> document::catalog() const noexcept
    {
        if (catalog_.has_value())
            return catalog_.value();

        auto parser_result = parser();
        if (!parser_result)
            return std::unexpected(parser_result.error());

        auto parsed = parser_result->get().catalog();
        if (!parsed)
            return std::unexpected(parsed.error());

        catalog_.emplace(std::move(*parsed));
        return catalog_.value();
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
