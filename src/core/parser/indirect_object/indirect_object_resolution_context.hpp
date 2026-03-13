#pragma once

#include "core/document.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"

namespace ripper::core
{
    class document;
    class cross_reference_table;
    class trailer;

    struct indirect_object_resolution_context
    {
        const document &doc;
        const cross_reference_table &xref_table;
        const trailer &trailer_dict;
    };
}
