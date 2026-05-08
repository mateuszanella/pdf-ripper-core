#include "core/parser/parser_manager.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/default_object_parser.hpp"
#include "core/parser/document_structure/default_document_structure_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
#include "core/parser/object_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"

namespace ripper::core
{
    parser_manager::parser_manager(document &doc)
        : document_{doc}
    {
    }

    parser_manager::~parser_manager() = default;

    void parser_manager::set_header_parser(std::unique_ptr<class header_parser> value) noexcept
    {
        header_parser_ = std::move(value);
    }

    void parser_manager::set_cross_reference_table_parser(std::unique_ptr<class cross_reference_table_parser> value) noexcept
    {
        xref_parser_ = std::move(value);
    }

    void parser_manager::set_trailer_parser(std::unique_ptr<class trailer_parser> value) noexcept
    {
        trailer_parser_ = std::move(value);
    }

    void parser_manager::set_document_structure_parser(std::unique_ptr<class document_structure_parser> value) noexcept
    {
        structure_parser_ = std::move(value);
    }

    void parser_manager::set_indirect_object_resolver(std::unique_ptr<class indirect_object_resolver> value) noexcept
    {
        object_resolver_ = std::move(value);
    }

    void parser_manager::set_object_parser(std::unique_ptr<class object_parser> value) noexcept
    {
        object_parser_ = std::move(value);
    }

    header_parser &parser_manager::header_parser()
    {
        if (!header_parser_)
            header_parser_ = std::make_unique<class header_parser>(document_);

        return *header_parser_;
    }

    cross_reference_table_parser &parser_manager::cross_reference_table_parser()
    {
        if (!xref_parser_)
            xref_parser_ = std::make_unique<class default_cross_reference_table_parser>();

        return *xref_parser_;
    }

    trailer_parser &parser_manager::trailer_parser()
    {
        if (!trailer_parser_)
            trailer_parser_ = std::make_unique<class default_trailer_parser>();

        return *trailer_parser_;
    }

    document_structure_parser &parser_manager::document_structure_parser()
    {
        if (!structure_parser_)
            structure_parser_ = std::make_unique<class default_document_structure_parser>(document_);

        return *structure_parser_;
    }

    indirect_object_resolver &parser_manager::object_resolver()
    {
        if (!object_resolver_)
            object_resolver_ = std::make_unique<class indirect_object_resolver>(document_);

        return *object_resolver_;
    }

    object_parser &parser_manager::object_parser()
    {
        if (!object_parser_)
            object_parser_ = std::make_unique<class default_object_parser>();

        return *object_parser_;
    }
}
