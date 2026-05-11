#include "core/document/object/indirect_object.hpp"

#include "core/document.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/error.hpp"

namespace ripper::pdf::core
{
    indirect_object::indirect_object(document &doc, indirect_reference ref) noexcept
        : document_{doc}, reference_{ref}
    {
    }

    const indirect_reference &indirect_object::reference() const noexcept
    {
        return reference_;
    }

    document &indirect_object::owner() const noexcept
    {
        return document_.get();
    }
}
