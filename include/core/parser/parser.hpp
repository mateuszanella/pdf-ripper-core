#pragma once

#include <expected>
#include <memory>
#include <vector>

#include "core/document/header.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class document;
    class parser_manager;
    class pages;
    class indirect_reference;

    /// Materialized document structure assembled from cross-reference and trailer chains.
    struct parsed_structure
    {
        /// Compiled (final) cross-reference table.
        class cross_reference_table compiled_xref;

        /// Historical cross-reference tables in traversal/merge order.
        std::vector<class cross_reference_table> xref_history;

        /// Compiled (final) trailer dictionary.
        class trailer compiled_trailer;

        /// Historical trailer dictionaries in traversal/merge order.
        std::vector<class trailer> trailer_history;
    };

    /// High-level PDF parser facade for a single `document`.
    ///
    /// This type orchestrates parsing by delegating to components managed by
    /// `parser_manager`, and returns failures through `std::expected<..., error>`.
    class parser
    {
    public:
        /// Construct a parser bound to `doc`.
        ///
        /// The parser stores a reference and does not take ownership of the document.
        explicit parser(const document &doc);

        /// Destroy the parser and its internal manager.
        ~parser();

        /// Return the parser manager used by this parser.
        ///
        /// Can be used to replace parser subcomponents.
        [[nodiscard]] parser_manager &manager();

        /// Parse and return the document header.
        [[nodiscard]] std::expected<header, error> header();

        /// Parse and return compiled document structure and traversal history.
        [[nodiscard]] std::expected<parsed_structure, error> structure();

        /// Parse and return the document catalog.
        [[nodiscard]] std::expected<catalog, error> catalog();

        /// Parse and return a pages tree rooted at `pages_ref`.
        [[nodiscard]] std::expected<pages, error> pages(indirect_reference pages_ref);

    private:
        const document &document_;
        std::unique_ptr<parser_manager> manager_;
    };
}
