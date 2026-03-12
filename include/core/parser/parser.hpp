#pragma once

#include <expected>
#include <functional>
#include <optional>
#include <vector>

#include "core/document/header.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class document;

    /**
     * @brief Parser facade for PDF documents.
     *
     * Eagerly parses xref + trailer (required for navigation).
     * Object-specific parsing is delegated to dedicated parser components.
     */
    class parser
    {
    public:
        explicit parser(const document &doc, reader &reader);

        [[nodiscard]] std::expected<void, parser_error> ensure_structure();

        [[nodiscard]] std::expected<header, parser_error> header();
        [[nodiscard]] std::expected<cross_reference_table, parser_error> cross_reference_table();
        [[nodiscard]] std::expected<trailer, parser_error> trailer();
        [[nodiscard]] std::expected<catalog, parser_error> catalog();

    private:
        const document &document_;
        reader &reader_;

        std::optional<class header> header_;
        std::optional<class cross_reference_table> xref_table_;
        std::optional<std::vector<class cross_reference_table>> xref_history_;
        std::optional<class trailer> trailer_;
        std::optional<std::vector<class trailer>> trailer_history_;

        std::optional<class catalog> catalog_;

        bool structure_parsed_ = false;

        [[nodiscard]] std::expected<void, parser_error> parse_header_if_needed();
        [[nodiscard]] std::expected<void, parser_error> parse_structure_if_needed();
    };
}
