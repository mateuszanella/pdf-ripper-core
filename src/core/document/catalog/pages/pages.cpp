#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>

#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    pages::pages(indirect_object &obj) noexcept
        : object_view(obj)
    {
    }

    std::uint64_t pages::count() const
    {
        auto *d = obj().dictionary();
        if (!d)
            throw parse_exception{"Pages content is not a dictionary"};

        auto count = d->get_integer("Count");
        if (!count)
        {
            throw parse_exception{"Pages indirect_object is missing required /Count entry"};
        }

        return static_cast<std::uint64_t>(*count);
    }
}
