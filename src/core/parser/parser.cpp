#include "core/parser/parser.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"

namespace ripper::core
{
    parser::parser(const document &doc)
        : document_{doc}
    {
    }

    void parser::set_header_parser(std::unique_ptr<class header_parser> value) noexcept
    {
        header_parser_ = std::move(value);
    }

    void parser::set_cross_reference_table_parser(std::unique_ptr<class cross_reference_table_parser> value) noexcept
    {
        xref_parser_ = std::move(value);
    }

    void parser::set_trailer_parser(std::unique_ptr<class trailer_parser> value) noexcept
    {
        trailer_parser_ = std::move(value);
    }

    void parser::set_indirect_object_resolver(std::unique_ptr<class indirect_object_resolver> value) noexcept
    {
        object_resolver_ = std::move(value);
    }

    void parser::set_catalog_parser(std::unique_ptr<class catalog_parser> value) noexcept
    {
        catalog_parser_ = std::move(value);
    }

    void parser::set_document_structure_parser(std::unique_ptr<class document_structure_parser> value) noexcept
    {
        structure_parser_ = std::move(value);
    }

    header_parser &parser::header_parser() const
    {
        if (!header_parser_)
            header_parser_ = std::make_unique<class header_parser>(document_.reader());

        return *header_parser_;
    }

    cross_reference_table_parser &parser::cross_reference_table_parser() const
    {
        if (!xref_parser_)
            xref_parser_ = std::make_unique<class default_cross_reference_table_parser>(document_.reader());

        return *xref_parser_;
    }

    trailer_parser &parser::trailer_parser() const
    {
        if (!trailer_parser_)
            trailer_parser_ = std::make_unique<class default_trailer_parser>(document_.reader());

        return *trailer_parser_;
    }

    document_structure_parser &parser::document_structure_parser() const
    {
        if (!structure_parser_)
            structure_parser_ = std::make_unique<class default_document_structure_parser>(
                document_.reader(),
                cross_reference_table_parser(),
                trailer_parser());

        return *structure_parser_;
    }

    catalog_parser &parser::catalog_parser() const
    {
        if (!catalog_parser_)
            catalog_parser_ = std::make_unique<class default_catalog_parser>(document_);

        return *catalog_parser_;
    }

    indirect_object_resolver &parser::object_resolver() const
    {
        if (!object_resolver_)
            object_resolver_ = std::make_unique<class indirect_object_resolver>(document_);

        return *object_resolver_;
    }

    std::expected<header, parser_error> parser::header()
    {
        return header_parser().parse();
    }

    std::expected<parsed_structure, parser_error> parser::structure()
    {
        auto result = document_structure_parser().parse();
        if (!result)
            return std::unexpected(result.error());

        parsed_structure out{
            .compiled_xref = std::move(result->compiled_xref),
            .xref_history = std::move(result->xref_history),
            .compiled_trailer = std::move(result->compiled_trailer),
            .trailer_history = std::move(result->trailer_history),
        };

        return out;
    }

    std::expected<catalog, parser_error> parser::catalog(const class trailer &tr)
    {
        auto root_obj = tr.root();
        if (!root_obj)
            return std::unexpected(parser_error::missing_catalog);

        auto content = object_resolver().resolve(root_obj.value());
        if (!content)
            return std::unexpected(content.error());

        return catalog{document_, root_obj.value()};
    }

    std::expected<catalog, parser_error> parser::catalog()
    {
        auto tr = trailer();
        if (!tr)
            return std::unexpected(tr.error());
        return catalog();
    }
}
