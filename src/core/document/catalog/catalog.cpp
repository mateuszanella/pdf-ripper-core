#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/object_view.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
catalog::catalog(indirect_object& obj) noexcept : object_view(obj) {}

class pages catalog::pages()
{
    auto* d = obj().dictionary();
    if (!d)
        throw parse_exception{"Catalog content is not a dictionary"};

    auto pages_ref = d->get_indirect_reference("Pages");
    if (!pages_ref)
        throw parse_exception{"Catalog is missing required /Pages reference"};

    auto* resolved = obj().identity().owner().resolve_object(*pages_ref);

    return ripper::pdf::core::pages{*resolved};
}

indirect_reference catalog::root_pages_indirect_reference()
{
    auto* d = obj().dictionary();
    if (!d)
        throw parse_exception{"Catalog content is not a dictionary"};

    const auto* pages_ref = d->get_indirect_reference("Pages");
    if (!pages_ref)
        throw parse_exception{"Catalog is missing required /Pages reference"};

    return *pages_ref;
}
} // namespace ripper::pdf::core
