#pragma once

#include <cstdint>
#include <functional>

#include "core/document/object/object.hpp"
#include "core/document/object/object_view.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// Typed view over a PDF pages tree object (/Type /Pages).
    ///
    /// Non-owning wrapper around an `object` stored in the cross-reference table.
    /// Ownership remains with the `cross_reference_entry` that holds the object.
    class pages : public object_view
    {
    public:
        explicit pages(object &obj) noexcept;

        /// Returns the total page count from the /Count entry.
        [[nodiscard]] std::uint64_t count() const;
    };
}
