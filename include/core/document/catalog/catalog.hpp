#pragma once

#include <functional>

#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// Typed view over the PDF document catalog (/Type /Catalog, the /Root object).
    ///
    /// Non-owning wrapper around an `object` stored in the cross-reference table.
    /// Ownership remains with the `cross_reference_entry` that holds the object.
    class catalog : public object_view
    {
    public:
        /// Construct a catalog view over an existing object.
        explicit catalog(object &obj) noexcept;

        /// Return a pages view for this catalog's page tree.
        ///
        /// Resolves the /Pages indirect reference through the owning document's
        /// cross-reference table, parsing and caching the object on first access.
        [[nodiscard]] class pages pages();
    };
}
