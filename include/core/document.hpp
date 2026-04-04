#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/parser.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class parser;
    class catalog;
    class header;
    class cross_reference_table;
    class trailer;

    class document
    {
    public:
        explicit document(std::unique_ptr<ripper::core::reader> reader);
        explicit document(const std::filesystem::path &path);

        static document open(const std::filesystem::path &path);

        [[nodiscard]] ripper::core::reader &reader() const noexcept;
        [[nodiscard]] ripper::core::parser &parser() const noexcept;

        [[nodiscard]] std::expected<header, parser_error> header() const;
        [[nodiscard]] std::expected<cross_reference_table, parser_error> cross_reference_table() const;
        [[nodiscard]] std::expected<trailer, parser_error> trailer() const;
        [[nodiscard]] std::expected<catalog, parser_error> catalog() const;

    private:
        std::unique_ptr<class reader> reader_;
        std::unique_ptr<class parser> parser_;

        mutable std::optional<class header> header_;

        mutable std::optional<class cross_reference_table> xref_table_;
        mutable std::optional<std::vector<class cross_reference_table>> xref_history_;

        mutable std::optional<class trailer> trailer_;
        mutable std::optional<std::vector<class trailer>> trailer_history_;

        mutable std::optional<class catalog> catalog_;
    };
}
