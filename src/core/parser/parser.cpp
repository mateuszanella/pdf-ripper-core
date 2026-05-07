#include "core/parser/parser.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/parser/parser_manager.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/parser/catalog/catalog_parser.hpp"
#include "core/parser/catalog/default_catalog_parser.hpp"
#include "core/parser/catalog/pages/pages_parser.hpp"
#include "core/parser/catalog/pages/default_pages_parser.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/document_structure/default_document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
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

        auto content = manager().object_resolver().resolve(*root_ref);
        if (!content)
            return std::unexpected(content.error());

        auto parse_result = manager().catalog_parser().parse(content.value());
        if (!parse_result)
            return std::unexpected(parse_result.error());

        class catalog c
        {
            object { indirect_object{document_, *root_ref}, std::move(parse_result.value()) }
        };

        return c;
    }

    std::expected<class pages, error> parser::pages(indirect_reference obj)
    {
        auto content = manager().object_resolver().resolve(obj);
        if (!content)
            return std::unexpected(content.error());

        auto parse_result = manager().pages_parser().parse(content.value());
        if (!parse_result)
            return std::unexpected(parse_result.error());

        class pages p
        {
            object { indirect_object{document_, obj}, std::move(parse_result.value()) }
        };

        return p;
    }
}
