#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <optional>
#include <string>

#include "core/document.hpp"
#include "core/document/catalog/pages/page/page.hpp"
#include "core/document/object/indirect_object.hpp"
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
            throw logic_exception{"Pages content is not a dictionary"};

        auto count = d->get_integer("Count");
        if (!count)
            throw logic_exception{"Pages indirect_object is missing required /Count entry"};

        return static_cast<std::uint64_t>(*count);
    }

    std::optional<class page> pages::page(std::uint64_t index)
    {
        auto *d = obj().dictionary();
        if (!d)
            throw logic_exception{"Pages content is not a dictionary"};

        auto kids = d->get_array("Kids");
        if (!kids)
            throw logic_exception{"Pages object is missing required /Kids entry"};

        if (index >= kids->size())
            return std::nullopt;

        auto kid_obj = (*kids)[index];

        auto *kid_indirect_ref = kid_obj.as_indirect_reference();
        if (!kid_indirect_ref)
            throw logic_exception{"Page index " + std::to_string(index) + " is not an indirect reference"};

        auto *resolved = obj().identity().owner().resolve_object(*kid_indirect_ref);
        if (!resolved)
            return std::nullopt;

        return ripper::pdf::core::page{*resolved};
    }

    std::optional<class page> pages::page(indirect_reference ref)
    {
        auto *resolved = obj().identity().owner().resolve_object(ref);
        if (!resolved)
            return std::nullopt;

        auto *d = resolved->dictionary();
        if (!d)
            throw logic_exception{"Page reference " + std::to_string(ref.object_number()) + " is not a dictionary"};

        auto type = d->get_name("Type");
        if (!type || type->value != "Page")
            throw logic_exception{"Page reference " + std::to_string(ref.object_number()) + " does not have /Type /Page"};

        return ripper::pdf::core::page{*resolved};
    }

    void pages::each(const std::function<void(class page &)> &callback)
    {
        if (!callback)
            throw logic_exception{"Pages::each callback cannot be empty"};

        const auto total = count();
        for (std::uint64_t index = 0; index < total; ++index)
        {
            auto current_page = page(index);
            if (!current_page)
                throw logic_exception{"Pages::each failed to resolve page at index " + std::to_string(index)};

            std::invoke(callback, *current_page);
        }
    }
}
