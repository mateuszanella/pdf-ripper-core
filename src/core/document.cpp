#include "core/document.hpp"

#include <fstream>
#include <memory>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
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

    std::expected<header, parser_error> document::header() const
    {
        return parser_->header();
    }

    std::expected<cross_reference_table, parser_error> document::cross_reference_table() const
    {
        return parser_->cross_reference_table();
    }

    std::expected<trailer, parser_error> document::trailer() const
    {
        return parser_->trailer();
    }

    std::expected<catalog, parser_error> document::catalog() const
    {
        return parser_->catalog();
    }
}
