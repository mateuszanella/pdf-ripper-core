#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    catalog::catalog(object obj) noexcept
        : object{std::move(obj)}
    {
    }

    std::expected<class pages, error> catalog::pages()
    {
        if (pages_.has_value())
            return pages_.value();

        pages_ref = dictionary().get_indirect_reference("Pages");
        if (!pages_ref)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_catalog)
                                       .with_component(error_component::catalog)
                                       .with_field("Pages")
                                       .with_expected("indirect reference to pages object")
                                       .with_message("Catalog is missing required /Pages reference")
                                       .build());
        }

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
