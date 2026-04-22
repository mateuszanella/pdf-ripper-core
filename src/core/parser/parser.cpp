#include "core/parser/parser.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/parser/parser_manager.hpp"
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
    parser::parser(const document &doc)
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

    std::expected<parsed_structure, error> parser::structure()
    {
        auto result = manager().document_structure_parser().parse();
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

    std::expected<catalog, error> parser::catalog()
    {
        const auto trailer = document_.trailer();
        if (!trailer)
            return std::unexpected(trailer.error());

        if (!trailer->root().has_value())
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_catalog)
                                       .with_component(error_component::trailer)
                                       .with_field("Root")
                                       .with_expected("indirect reference")
                                       .with_actual("missing")
                                       .with_message("Trailer does not contain a Root reference")
                                       .build());

        const auto root_ref = trailer->root().value();

        auto content = manager().object_resolver().resolve(root_ref);
        if (!content)
            return std::unexpected(content.error());

        auto parsed = manager().catalog_parser().parse(content.value());
        if (!parsed)
            return std::unexpected(parsed.error());

        class catalog out{document_, root_ref, parsed->pages_ref};

        return out;
    }

    std::expected<class pages, error> parser::pages(indirect_reference obj)
    {
        auto content = manager().object_resolver().resolve(obj);
        if (!content)
            return std::unexpected(content.error());

        auto parsed = manager().pages_parser().parse(content.value());
        if (!parsed)
            return std::unexpected(parsed.error());

        class pages out{document_, obj, parsed->count};

        return out;
    }
}
