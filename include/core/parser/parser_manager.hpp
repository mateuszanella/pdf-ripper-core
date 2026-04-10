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

    class parser_manager
    {
    public:
        explicit parser_manager(const document &doc);
        ~parser_manager();

        void set_header_parser(std::unique_ptr<class header_parser> value) noexcept;
        void set_cross_reference_table_parser(std::unique_ptr<class cross_reference_table_parser> value) noexcept;
        void set_trailer_parser(std::unique_ptr<class trailer_parser> value) noexcept;
        void set_catalog_parser(std::unique_ptr<class catalog_parser> value) noexcept;
        void set_pages_parser(std::unique_ptr<class pages_parser> value) noexcept;
        void set_document_structure_parser(std::unique_ptr<class document_structure_parser> value) noexcept;
        void set_indirect_object_resolver(std::unique_ptr<class indirect_object_resolver> value) noexcept;

        [[nodiscard]] class header_parser &header_parser();
        [[nodiscard]] class cross_reference_table_parser &cross_reference_table_parser();
        [[nodiscard]] class trailer_parser &trailer_parser();
        [[nodiscard]] class catalog_parser &catalog_parser();
        [[nodiscard]] class pages_parser &pages_parser();
        [[nodiscard]] class document_structure_parser &document_structure_parser();
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
