#include "core/document/catalog/pages/page_tree_node.hpp"

#include <algorithm>
#include <cstdint>

#include "core/document.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/document/catalog/pages/page/page.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    page_tree_node::page_tree_node(indirect_object &obj) noexcept
        : object_view(obj)
    {
    }

    bool page_tree_node::is_leaf() const
    {
        const auto *d = obj().dictionary();
        if (!d)
            return false;

        const auto *type = d->get_name("Type");
        return type && type->value == "Page";
    }

    bool page_tree_node::is_root() const
    {
        const auto root_ref = obj().identity().owner().catalog().root_pages_indirect_reference();
        return root_ref == obj().identity().reference();
    }

    std::optional<page_tree_node> page_tree_node::parent()
    {
        const auto *d = obj().dictionary();
        if (!d)
            return std::nullopt;

        const auto *parent_ref = d->get_indirect_reference("Parent");
        if (!parent_ref)
            return std::nullopt;

        auto *resolved = obj().identity().owner().resolve_object(*parent_ref);
        if (!resolved)
            return std::nullopt;

        return page_tree_node{*resolved};
    }

    std::vector<page_tree_node> page_tree_node::children()
    {
        const auto *d = obj().dictionary();
        if (!d)
            return {};

        const auto *kids = d->get_array("Kids");
        if (!kids)
            return {};

        std::vector<page_tree_node> result;
        result.reserve(kids->size());

        for (const auto &kid_obj : *kids)
        {
            const auto *ref = kid_obj.as_indirect_reference();
            if (!ref)
                throw logic_exception{"Kid entry is not an indirect reference"};

            auto *resolved = obj().identity().owner().resolve_object(*ref);
            if (!resolved)
                throw logic_exception{"Failed to resolve kid reference"};

            result.emplace_back(*resolved);
        }

        return result;
    }

    void page_tree_node::remove_child(const indirect_reference &ref)
    {
        auto *d = obj().dictionary();
        if (!d)
            throw logic_exception{"Page tree node content is not a dictionary"};

        auto *kids = d->get_array("Kids");
        if (!kids)
            throw logic_exception{"Pages node is missing required /Kids entry"};

        auto it = std::find_if(kids->begin(), kids->end(), [&](const object &o)
                               {
            const auto *r = o.as_indirect_reference();
            return r && *r == ref; });

        if (it != kids->end())
            kids->erase(it);
    }

    std::optional<class pages> page_tree_node::as_pages()
    {
        const auto *d = obj().dictionary();
        if (!d)
            return std::nullopt;

        const auto *type = d->get_name("Type");
        if (!type || type->value != "Pages")
            return std::nullopt;

        return pages{obj()};
    }

    std::optional<class page> page_tree_node::as_page()
    {
        const auto *d = obj().dictionary();
        if (!d)
            return std::nullopt;

        const auto *type = d->get_name("Type");
        if (!type || type->value != "Page")
            return std::nullopt;

        return page{obj()};
    }

    std::uint64_t page_tree_node::subtree_count() const
    {
        const auto *d = obj().dictionary();
        if (!d)
            throw logic_exception{"Page tree node content is not a dictionary"};

        const auto *type = d->get_name("Type");
        if (type && type->value == "Page")
            return 1;

        const auto *count = d->get_integer("Count");
        if (!count)
            throw logic_exception{"Pages node is missing required /Count entry"};

        return static_cast<std::uint64_t>(*count);
    }
}
