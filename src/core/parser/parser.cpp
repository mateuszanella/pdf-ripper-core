#include "core/parser/parser.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
// #include "core/parser/catalog/catalog_parser.hpp"
// #include "core/parser/catalog/default_catalog_parser.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/document_structure/default_document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
#include "core/parser/trailer/trailer_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"

namespace ripper::core
{
    parser::parser(const document &doc)
        : document_{doc}
    {
    }

    parser::~parser() = default;

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

    // void parser::set_catalog_parser(std::unique_ptr<class catalog_parser> value) noexcept
    // {
    //     catalog_parser_ = std::move(value);
    // }

    void parser::set_document_structure_parser(std::unique_ptr<class document_structure_parser> value) noexcept
    {
        structure_parser_ = std::move(value);
    }

    header_parser &parser::header_parser()
    {
        if (!header_parser_)
            header_parser_ = std::make_unique<class header_parser>(document_.reader());

        return *header_parser_;
    }

    cross_reference_table_parser &parser::cross_reference_table_parser()
    {
        if (!xref_parser_)
            xref_parser_ = std::make_unique<class default_cross_reference_table_parser>();

        return *xref_parser_;
    }

    trailer_parser &parser::trailer_parser()
    {
        if (!trailer_parser_)
            trailer_parser_ = std::make_unique<class default_trailer_parser>();

        return *trailer_parser_;
    }

    document_structure_parser &parser::document_structure_parser()
    {
        if (!structure_parser_)
            structure_parser_ = std::make_unique<class default_document_structure_parser>(
                document_.reader());

        return *structure_parser_;
    }

    // catalog_parser &parser::catalog_parser()
    // {
    //     if (!catalog_parser_)
    //         catalog_parser_ = std::make_unique<class default_catalog_parser>();

    //     return *catalog_parser_;
    // }

    indirect_object_resolver &parser::object_resolver()
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

    // std::expected<catalog, parser_error> parser::catalog(const class trailer &tr)
    // {
    //     auto root_obj = tr.root();
    //     if (!root_obj)
    //         return std::unexpected(parser_error::missing_catalog);

    //     auto content = object_resolver().resolve(root_obj.value());
    //     if (!content)
    //         return std::unexpected(content.error());

    //     return catalog{document_, root_obj.value()};
    // }
}
