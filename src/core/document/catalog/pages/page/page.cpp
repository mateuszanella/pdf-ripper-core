#include "core/document/catalog/pages/page/page.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    page::page(indirect_object &obj) noexcept
        : object_view(obj)
    {
    }

    std::optional<class pages> page::parent()
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

        const auto *parent_dict = resolved->dictionary();
        if (!parent_dict)
            throw logic_exception{"Page /Parent is not a dictionary"};

        const auto *type = parent_dict->get_name("Type");
        if (!type || type->value != "Pages")
            throw logic_exception{"Page /Parent does not have /Type /Pages"};

        return pages{*resolved};
    }
}
