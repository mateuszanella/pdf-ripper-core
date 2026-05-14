#include "core/document/object/object_view.hpp"

namespace ripper::pdf::core
{
    object_view::object_view(object &obj) noexcept : obj_{obj}
    {
    }

    object &object_view::obj() noexcept
    {
        return obj_.get();
    }

    const object &object_view::obj() const noexcept
    {
        return obj_.get();
    }

    class dictionary *object_view::dictionary() noexcept
    {
        return obj().dictionary();
    }

    const class dictionary *object_view::dictionary() const noexcept
    {
        return obj().dictionary();
    }
}
