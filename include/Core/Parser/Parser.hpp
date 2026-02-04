#pragma once

#include <expected>
#include <optional>
#include <vector>

#include "Core/Document/Header.hpp"
#include "Core/Document/CrossReferenceTable/CrossReferenceTable.hpp"
#include "Core/Document/Trailer/Trailer.hpp"
#include "Core/Document/Catalog/Catalog.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    /**
     * @brief High-level parser orchestrator for PDF documents.
     *
     * Provides lazy-loading accessors for document components.
     * Parsing is deferred until first access unless EnsureParsed() is called.
     */
    class Parser
    {
    public:
        explicit Parser(Reader &reader);

        /**
         * @brief Ensures all PDF structures are parsed.
         * Safe to call multiple times - parsing only happens once.
         */
        [[nodiscard]] std::expected<void, ParserError> EnsureParsed();

        /**
         * @brief Returns the PDF header. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<Header, ParserError> Header();

        /**
         * @brief Returns the compiled cross-reference table. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<CrossReferenceTable, ParserError> CrossReferenceTable();

        /**
         * @brief Returns all cross-reference tables found in the document (newest to oldest).
         * Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<std::vector<class CrossReferenceTable>, ParserError> CrossReferenceTableHistory();

        /**
         * @brief Returns the compiled trailer. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<Trailer, ParserError> Trailer();

        /**
         * @brief Returns all trailers found in the document (newest to oldest).
         * Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<std::vector<class Trailer>, ParserError> TrailerHistory();

        /**
         * @brief Returns the document catalog. Triggers parsing if not yet done.
         */
        [[nodiscard]] std::expected<Catalog, ParserError> Catalog();

    private:
        Reader &_reader;

        std::optional<class Header> _header;
        std::optional<class CrossReferenceTable> _compiledXrefTable;
        std::optional<std::vector<class CrossReferenceTable>> _xrefTableHistory;
        std::optional<class Trailer> _compiledTrailer;
        std::optional<std::vector<class Trailer>> _trailerHistory;
        std::optional<class Catalog> _catalog;

        bool _structureParsed = false;

        [[nodiscard]] std::expected<void, ParserError> ParseHeaderIfNeeded();
        [[nodiscard]] std::expected<void, ParserError> ParseStructureIfNeeded();
        [[nodiscard]] std::expected<void, ParserError> ParseCatalogIfNeeded();
    };
}
