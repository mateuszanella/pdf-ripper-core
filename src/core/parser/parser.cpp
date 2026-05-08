#include "core/parser/parser.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/parser/parser_manager.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/document_structure/default_document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
#include "core/parser/object_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    parser::parser(document &doc)
        : document_{doc},
          manager_{std::make_unique<class parser_manager>(doc)}
    {
    }

    parser::~parser() = default;

    parser_manager &parser::manager()
    {
        if (!manager_)
            manager_ = std::make_unique<class parser_manager>(document_);

        return *manager_;
    }

    std::expected<header, error> parser::header()
    {
        return manager().header_parser().parse();
    }

    std::expected<document_structure, error> parser::structure()
    {
        return manager().document_structure_parser().parse();
    }

    std::expected<catalog, error> parser::catalog()
    {
        const auto trailer = document_.trailer();
        if (!trailer)
            return std::unexpected(trailer.error());

        auto root_ref = trailer->get().root();
        if (!root_ref)
            return std::unexpected(root_ref.error());

        auto obj = parse_object(*root_ref);
        if (!obj)
            return std::unexpected(obj.error());

        return ripper::core::catalog{std::move(*obj)};
    }

    std::expected<class pages, error> parser::pages(indirect_reference ref)
    {
        auto obj = parse_object(ref);
        if (!obj)
            return std::unexpected(obj.error());

        return ripper::core::pages{std::move(*obj)};
    }

    std::expected<object, error> parser::parse_object(indirect_reference ref)
    {
        auto content = manager().object_resolver().resolve(ref);
        if (!content)
            return std::unexpected(content.error());

        return manager().object_parser().parse(document_, ref, *content);
    }
}
