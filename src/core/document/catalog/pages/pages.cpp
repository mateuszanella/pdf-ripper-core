#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <memory>
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

    class page pages::add_page()
    {
        auto &doc = obj().identity().owner();
        auto &xref = doc.cross_reference_table();

        const auto page_ref = xref.reserve();

        // This should in fact be a parameter
        array mediabox{
            object{std::int64_t{0}},
            object{std::int64_t{0}},
            object{std::int64_t{612}},
            object{std::int64_t{792}}};

        ripper::pdf::core::dictionary page_dict;
        page_dict.set("Type", object{name{"Page"}});
        page_dict.set("Parent", object{obj().identity().reference()});
        page_dict.set("MediaBox", object{std::move(mediabox)});

        auto page_obj = std::make_unique<indirect_object>(
            object_identity{&doc, page_ref}, object{std::move(page_dict)});

        auto *raw_page = xref.commit(page_ref, std::move(page_obj));
        if (!raw_page)
            throw logic_exception{"Failed to commit page to cross-reference table"};

        // Update /Kids and /Count on this pages node
        auto *d = obj().dictionary();
        if (!d)
            throw logic_exception{"Pages content is not a dictionary"};

        if (!d->get_array("Kids"))
            d->set("Kids", object{array{}});

        auto *kids = d->get_array("Kids");
        kids->push_back(object{page_ref});

        std::uint64_t count = 0;

        const auto *count_ptr = d->get_integer("Count");
        if (count_ptr)
            count = static_cast<std::uint64_t>(*count_ptr);

        d->set("Count", object{static_cast<std::int64_t>(count + 1)});

        return ripper::pdf::core::page{*raw_page};
    }
}
