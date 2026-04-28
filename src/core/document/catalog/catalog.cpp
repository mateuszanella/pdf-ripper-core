#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    catalog::catalog(const document &doc, indirect_reference ref, std::optional<indirect_reference> pages_ref) noexcept
        : indirect_object{doc, ref}, pages_ref_{pages_ref}
    {
    }

    catalog::catalog(indirect_object obj, std::optional<indirect_reference> pages_ref) noexcept
        : indirect_object{std::move(obj)}, pages_ref_{pages_ref}
    {
    }

    std::expected<class pages, error> catalog::pages()
    {
        if (pages_.has_value())
            return pages_.value();

        if (!pages_ref_.has_value())
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::missing_catalog)
                                       .with_component(error_component::catalog)
                                       .with_field("Pages")
                                       .with_expected("indirect reference")
                                       .with_actual("missing")
                                       .with_message("Catalog does not contain a Pages reference")
                                       .build());

        auto parser = owner().parser();
        if (!parser)
            return std::unexpected(parser.error());

        auto parsed = parser->get().pages(*pages_ref_);
        if (!parsed)
            return std::unexpected(parsed.error());

        pages_.emplace(std::move(parsed.value()));

        return pages_.value();
    }
}
