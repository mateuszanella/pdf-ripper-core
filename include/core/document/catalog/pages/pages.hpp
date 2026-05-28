#pragma once

#include <cstdint>
#include <functional>

#include "core/document/catalog/pages/page/page.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/object_view.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// Typed view over a PDF pages tree indirect_object (/Type /Pages).
    ///
    /// Non-owning wrapper around an `indirect_object` stored in the cross-reference table.
    /// Ownership remains with the `cross_reference_entry` that holds the indirect_object.
    class pages : public object_view
    {
    public:
        explicit pages(indirect_object &obj) noexcept;

        /// Returns the total page count from the /Count entry.
        [[nodiscard]] std::uint64_t count() const;

        /// Returns an optional page view for the page at `index` in the page tree, or
        /// `std::nullopt` if the page does not exist.
        ///
        /// The index starts at 0 and must be less than the total page count. Pages are
        /// ordered according to a depth-first traversal of the page tree.
        [[nodiscard]] std::optional<class page> page(std::uint64_t index);

        /// Returns an optional page view for the page based on an indirect reference,
        /// or `std::nullopt` if the page does not exist.
        [[nodiscard]] std::optional<class page> page(indirect_reference ref);

        /// Executes `callback` for each page in index order.
        ///
        /// Throws if callback is empty or if an expected page cannot be resolved.
        void each(const std::function<void(class page &)> &callback);

        /// Creates a new page and adds it to the end of the page tree.
        ///
        /// Also updates the /Kids and /Count entries on this pages node.
        /// The new page is returned as a view.
        [[nodiscard]] class page add_page();
    };
}
