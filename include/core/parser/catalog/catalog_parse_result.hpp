#pragma once

#include "core/document/indirect_reference.hpp"

namespace ripper::core
{
    /**
     * @brief Result of parsing a catalog object's raw content.
     */
    struct catalog_parse_result
    {
        indirect_reference pages_ref;
    };
}
