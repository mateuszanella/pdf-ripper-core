#pragma once

#include <memory>

namespace ripper::core
{
    class document;
    class header_parser;
    class cross_reference_table_parser;
    class trailer_parser;
    class catalog_parser;
    class pages_parser;
    class document_structure_parser;
    class indirect_object_resolver;

    /// Owns and exposes the parser subcomponents used to process a `document`.
    ///
    /// This type centralizes parser dependencies and enables runtime injection
    /// of concrete parser implementations (useful for composition and testing).
    /// All injected components are owned via `std::unique_ptr`.
    class parser_manager
    {
    public:
        /// Construct a manager bound to `doc`.
        ///
        /// The manager stores a reference and does not take ownership of the document.
        explicit parser_manager(const document &doc);

        /// Destroy the manager and all owned parser components.
        ~parser_manager();

        /// Replace the header parser implementation.
        void set_header_parser(std::unique_ptr<class header_parser> value) noexcept;

        /// Replace the cross-reference-table parser implementation.
        void set_cross_reference_table_parser(std::unique_ptr<class cross_reference_table_parser> value) noexcept;

        /// Replace the trailer parser implementation.
        void set_trailer_parser(std::unique_ptr<class trailer_parser> value) noexcept;

        /// Replace the catalog parser implementation.
        void set_catalog_parser(std::unique_ptr<class catalog_parser> value) noexcept;

        /// Replace the pages parser implementation.
        void set_pages_parser(std::unique_ptr<class pages_parser> value) noexcept;

        /// Replace the document-structure parser implementation.
        void set_document_structure_parser(std::unique_ptr<class document_structure_parser> value) noexcept;

        /// Replace the indirect-object resolver implementation.
        void set_indirect_object_resolver(std::unique_ptr<class indirect_object_resolver> value) noexcept;

        /// Access the configured header parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class header_parser &header_parser();

        /// Access the configured cross-reference-table parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class cross_reference_table_parser &cross_reference_table_parser();

        /// Access the configured trailer parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class trailer_parser &trailer_parser();

        /// Access the configured catalog parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class catalog_parser &catalog_parser();

        /// Access the configured pages parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class pages_parser &pages_parser();

        /// Access the configured document-structure parser.
        ///
        /// The corresponding parser must be configured before calling.
        [[nodiscard]] class document_structure_parser &document_structure_parser();

        /// Access the configured indirect-object resolver.
        ///
        /// The corresponding resolver must be configured before calling.
        [[nodiscard]] class indirect_object_resolver &object_resolver();

    private:
        const document &document_;

        std::unique_ptr<class header_parser> header_parser_;
        std::unique_ptr<class cross_reference_table_parser> xref_parser_;
        std::unique_ptr<class trailer_parser> trailer_parser_;
        std::unique_ptr<class catalog_parser> catalog_parser_;
        std::unique_ptr<class pages_parser> pages_parser_;
        std::unique_ptr<class document_structure_parser> structure_parser_;
        std::unique_ptr<class indirect_object_resolver> object_resolver_;
    };
}
