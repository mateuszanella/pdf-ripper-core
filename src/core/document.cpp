#include "core/document.hpp"

#include <fstream>
#include <memory>
#include <utility>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/file_reader.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    document::document(std::unique_ptr<class reader> reader)
        : reader_(std::move(reader)),
          parser_(std::make_unique<class parser>(*this))
    {
    }

    document::document(const std::filesystem::path &path)
        : document(std::make_unique<ripper::core::file_reader>(path))
    {
    }

    document document::open(const std::filesystem::path &path)
    {
        return document{path};
    }

    reader &document::reader() const noexcept
    {
        return *reader_;
    }

    parser &document::parser() const noexcept
    {
        return *parser_;
    }

    std::expected<header, error> document::header() const
    {
        if (header_.has_value())
            return header_.value();

        auto parsed = parser_->header();
        if (!parsed)
            return std::unexpected(parsed.error());

        header_ = std::move(*parsed);

        return header_.value();
    }

    std::expected<cross_reference_table, error> document::cross_reference_table() const
    {
        if (xref_table_.has_value())
            return xref_table_.value();

        auto parsed = parser_->structure();
        if (!parsed)
            return std::unexpected(parsed.error());

        xref_table_ = std::move(parsed->compiled_xref);
        xref_history_ = std::move(parsed->xref_history);
        trailer_ = std::move(parsed->compiled_trailer);
        trailer_history_ = std::move(parsed->trailer_history);

        return xref_table_.value();
    }

    std::expected<trailer, error> document::trailer() const
    {
        if (trailer_.has_value())
            return trailer_.value();

        auto parsed = parser_->structure();
        if (!parsed)
            return std::unexpected(parsed.error());

        xref_table_ = std::move(parsed->compiled_xref);
        xref_history_ = std::move(parsed->xref_history);
        trailer_ = std::move(parsed->compiled_trailer);
        trailer_history_ = std::move(parsed->trailer_history);

        return trailer_.value();
    }

    std::expected<catalog, error> document::catalog() const
    {
        if (catalog_.has_value())
            return catalog_.value();

        auto parsed = parser_->catalog();
        if (!parsed)
            return std::unexpected(parsed.error());

        catalog_.emplace(std::move(*parsed));

        return catalog_.value();
    }
}
