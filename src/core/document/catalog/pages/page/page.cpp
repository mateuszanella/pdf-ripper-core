#include "core/document/catalog/pages/page/page.hpp"

#include "core/document/object/indirect_object.hpp"

namespace ripper::pdf::core
{
    page::page(indirect_object &obj) noexcept
        : object_view(obj)
    {
    }
}
