#pragma once

#include <expected>
#include <functional>

#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/object.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /// Typed view over the PDF document catalog (/Type /Catalog, the /Root object).
    ///
    /// Non-owning wrapper around an `object` stored in the cross-reference table.
    /// Ownership remains with the `cross_reference_entry` that holds the object.
    class catalog
    {
    public:
        /// Construct a catalog view over an existing object.
        explicit catalog(object &obj) noexcept;

        /// Returns the underlying object.
        [[nodiscard]] object &obj() noexcept;
        [[nodiscard]] const object &obj() const noexcept;

        /// Returns a pointer to the content dictionary, or `nullptr` if content is not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

        /// Return a pages view for this catalog's page tree.
        ///
        /// Resolves the /Pages indirect reference through the owning document's
        /// cross-reference table, parsing and caching the object on first access.
        [[nodiscard]] std::expected<class pages, error> pages();

    private:
        std::reference_wrapper<object> obj_;
    };
}
