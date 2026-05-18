#pragma once

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/object_view.hpp"

namespace ripper::pdf::core
{
    class page : public object_view
    {
    public:
        explicit page(indirect_object &obj) noexcept;
    };
}
