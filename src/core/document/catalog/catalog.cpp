#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    catalog::catalog(object &obj)
        : obj_{obj}
    {
    }

    object &catalog::obj()
    {
        return obj_.get();
    }

    const object &catalog::obj() const
    {
        return obj_.get();
    }

    dictionary *catalog::dictionary()
    {
        return obj_.get().dictionary();
    }

    const dictionary *catalog::dictionary() const
    {
        return obj_.get().dictionary();
    }

    class pages catalog::pages()
    {
        auto *d = obj_.get().dictionary();
        if (!d)
            throw parse_exception{"Catalog content is not a dictionary"};

        auto pages_ref = d->get_indirect_reference("Pages");
        if (!pages_ref)
            throw parse_exception{"Catalog is missing required /Pages reference"};

        auto *resolved = obj_.get().identity().owner().resolve_object(*pages_ref);

        return ripper::pdf::core::pages{*resolved};
    }
}
