#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>

#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    pages::pages(object &obj)
        : obj_{obj}
    {
    }

    object &pages::obj()
    {
        return obj_.get();
    }

    const object &pages::obj() const
    {
        return obj_.get();
    }

    dictionary *pages::dictionary()
    {
        return obj_.get().dictionary();
    }

    const dictionary *pages::dictionary() const
    {
        return obj_.get().dictionary();
    }

    std::uint64_t pages::count() const
    {
        auto *d = obj_.get().dictionary();
        if (!d)
            throw parse_exception{"Pages content is not a dictionary"};

        auto count = d->get_integer("Count");
        if (!count)
        {
            throw parse_exception{"Pages object is missing required /Count entry"};
        }

        return static_cast<std::uint64_t>(*count);
    }
}
