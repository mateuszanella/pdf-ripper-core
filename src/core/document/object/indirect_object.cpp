#include "core/document/object/indirect_object.hpp"

#include "core/document.hpp"
#include "core/document/object/indirect_reference.hpp"

namespace ripper::pdf::core
{
    indirect_object::indirect_object(document &doc, indirect_reference ref)
        : document_{doc}, reference_{ref}
    {
    }

    const indirect_reference &indirect_object::reference() const
    {
        return reference_;
    }

    document &indirect_object::owner() const
    {
        return document_.get();
    }
}
