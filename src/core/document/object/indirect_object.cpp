#include "core/document/object/indirect_object.hpp"

#include "core/document.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    indirect_object::indirect_object(const document &doc, indirect_reference ref) noexcept
        : document_{doc}, reference_{ref}
    {
    }

    const indirect_reference &indirect_object::reference() const noexcept
    {
        return reference_;
    }

    const document &indirect_object::owner() const noexcept
    {
        return document_;
    }
}
