#pragma once

#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/exceptions/exception.hpp"

#include <functional>

namespace ripper::pdf::core
{
/// Typed view over the PDF document catalog (/Type /Catalog, the /Root indirect_object).
///
/// Non-owning wrapper around an `indirect_object` stored in the cross-reference table.
/// Ownership remains with the `cross_reference_entry` that holds the indirect_object.
class catalog : public object_view
{
public:
    /// Construct a catalog view over an existing indirect_object.
    explicit catalog(indirect_object& obj) noexcept;

    /// Return a pages view for this catalog's page tree.
    ///
    /// Resolves the /Pages indirect reference through the owning document's
    /// cross-reference table, parsing and caching the indirect_object on first access.
    [[nodiscard]] class pages pages();

    /// Returns the indirect reference to the root /Pages object in this catalog.
    ///
    /// This is the value of the /Pages entry in the catalog dictionary and
    /// identifies the root of the document's page tree.
    [[nodiscard]] indirect_reference root_pages_indirect_reference();
};
} // namespace ripper::pdf::core
