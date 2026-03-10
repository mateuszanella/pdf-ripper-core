#pragma once

#include <expected>
#include <optional>
#include <vector>

#include "core/document/Header.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    /**
     * @brief High-level parser orchestrator for pdf documents.
     *
     * Provides lazy-loading accessors for document components.
     * Parsing is deferred until first access unless ensure_parsed() is called.
     */
    class parser
    {
    public:
        explicit parser(reader &reader);

        /**
         * @brief Ensures all pdf structures are parsed.
         * Safe to call multiple times - parsing only happens once.
         */
        [[nodiscard]] std::expected<void, parser_error> ensure_parsed();

        /**
         * @brief Returns the pdf header. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<header, parser_error> header();

        /**
         * @brief Returns the compiled cross-reference table. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<cross_reference_table, parser_error> cross_reference_table();

        /**
         * @brief Returns all cross-reference tables found in the document (newest to oldest).
         * Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<std::vector<class cross_reference_table>, parser_error> cross_reference_table_history();

        /**
         * @brief Returns the compiled trailer. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<trailer, parser_error> trailer();

        /**
         * @brief Returns all trailers found in the document (newest to oldest).
         * Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<std::vector<class trailer>, parser_error> trailer_history();

        /**
         * @brief Returns the document catalog. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<catalog, parser_error> catalog();

    private:
        reader &_reader;

        std::optional<class header> _header;
        std::optional<class cross_reference_table> _compiledXrefTable;
        std::optional<std::vector<class cross_reference_table>> _xrefTableHistory;
        std::optional<class trailer> _compiledTrailer;
        std::optional<std::vector<class trailer>> _trailerHistory;
        std::optional<class catalog> _catalog;

        bool _structureParsed = false;

        [[nodiscard]] std::expected<void, parser_error> parse_header_if_needed();
        [[nodiscard]] std::expected<void, parser_error> parse_structure_if_needed();
        [[nodiscard]] std::expected<void, parser_error> parse_catalog_if_needed();
    };
}
