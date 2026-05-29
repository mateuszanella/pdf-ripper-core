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
    namespace
    {
        /// Resolves every entry in the /Kids array of `node` and dispatches to one of two
        /// typed callbacks based on the /Type of each resolved object:
        ///   - on_page(indirect_object &)                      — called for /Type /Page leaves
        ///   - on_pages(indirect_object &, const dictionary &) — called for /Type /Pages nodes
        /// Either callback may return false to stop iteration early.
        /// All resolution and validation errors throw; unknown /Type values also throw.
        template <typename OnPage, typename OnPages>
        void for_each_kid(indirect_object &node, OnPage on_page, OnPages on_pages)
        {
            const auto *d = node.dictionary();
            if (!d)
                throw logic_exception{"Pages node content is not a dictionary"};

            const auto *kids = d->get_array("Kids");
            if (!kids)
                throw logic_exception{"Pages node is missing required /Kids entry"};

            for (const auto &kid_obj : *kids)
            {
                const auto *ref = kid_obj.as_indirect_reference();
                if (!ref)
                    throw logic_exception{"Kid entry is not an indirect reference"};

                auto *resolved = node.identity().owner().resolve_object(*ref);
                if (!resolved)
                    throw logic_exception{"Failed to resolve kid reference"};

                const auto *kid_dict = resolved->dictionary();
                if (!kid_dict)
                    throw logic_exception{"Kid is not a dictionary"};

                const auto *type_name = kid_dict->get_name("Type");
                if (!type_name)
                    throw logic_exception{"Kid is missing /Type entry"};

                if (type_name->value == "Page")
                {
                    if (!on_page(*resolved))
                        return;
                }
                else if (type_name->value == "Pages")
                {
                    if (!on_pages(*resolved, *kid_dict))
                        return;
                }
                else
                {
                    throw logic_exception{"Kid has unexpected /Type: " + type_name->value};
                }
            }
        }

        /// DFS for the nth leaf /Page in the subtree rooted at `node`. `index` is decremented
        /// for each /Page leaf visited; when it reaches 0 the matching page is returned.
        /// The caller must ensure index < subtree count.
        std::optional<page> find_page_by_index(indirect_object &node, std::uint64_t &index)
        {
            std::optional<page> result;

            for_each_kid(
                node,
                [&](indirect_object &kid) -> bool
                {
                    if (index == 0)
                    {
                        result = page{kid};
                        return false;
                    }
                    --index;
                    return true;
                },
                [&](indirect_object &kid, const dictionary &kid_dict) -> bool
                {
                    const auto *count_ptr = kid_dict.get_integer("Count");
                    if (!count_ptr)
                        throw logic_exception{"Intermediate Pages node is missing /Count entry"};

                    const auto subtree_count = static_cast<std::uint64_t>(*count_ptr);
                    if (index < subtree_count)
                    {
                        result = find_page_by_index(kid, index);
                        return false;
                    }
                    index -= subtree_count;
                    return true;
                });

            return result;
        }

        /// DFS of the subtree rooted at `node`, invoking `cb` for every leaf /Page in document order.
        void walk_each(indirect_object &node, const std::function<void(page &)> &cb)
        {
            for_each_kid(
                node,
                [&](indirect_object &kid) -> bool
                {
                    page pg{kid};
                    cb(pg);
                    return true;
                },
                [&](indirect_object &kid, const dictionary &) -> bool
                {
                    walk_each(kid, cb);
                    return true;
                });
        }
    }

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
        if (index >= count())
            return std::nullopt;

        std::uint64_t remaining = index;
        return find_page_by_index(obj(), remaining);
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

        walk_each(obj(), callback);
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

    void pages::delete_page(std::uint64_t page_index)
    {
        auto page = this->page(page_index);
        if (!page)
            return;

        // Should walk up the page tree and decrement the Count of all parents.

        const auto &page_ref = page->obj().identity().reference();

        obj().identity().owner().cross_reference_table().find(page_ref)->mark_deleted();
    }

    void pages::prune_page(std::uint64_t page_index)
    {
        throw logic_exception{"Not implemented: pages::prune_page"};
    }
}
