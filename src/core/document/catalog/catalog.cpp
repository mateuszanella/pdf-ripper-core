#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::pdf::core
{
    catalog::catalog(object &obj) noexcept
        : obj_{obj}
    {
    }

    object &catalog::obj() noexcept
    {
        return obj_.get();
    }

    const object &catalog::obj() const noexcept
    {
        return obj_.get();
    }

    dictionary *catalog::dictionary() noexcept
    {
        return obj_.get().dictionary();
    }

    const dictionary *catalog::dictionary() const noexcept
    {
        return obj_.get().dictionary();
    }

    std::expected<class pages, error> catalog::pages()
    {
        auto *d = obj_.get().dictionary();
        if (!d)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_catalog)
                                       .with_component(error_component::catalog)
                                       .with_message("Catalog content is not a dictionary")
                                       .build());

        auto pages_ref = d->get_indirect_reference("Pages");
        if (!pages_ref)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_catalog)
                                       .with_component(error_component::catalog)
                                       .with_field("Pages")
                                       .with_expected("indirect reference to pages object")
                                       .with_message("Catalog is missing required /Pages reference")
                                       .build());

        auto result = obj_.get().identity().owner().resolve_object(*pages_ref);
        if (!result)
            return std::unexpected(result.error());

        return ripper::pdf::core::pages{**result};
    }
}
