#pragma once

#include <cstdint>
#include <expected>
#include <functional>

#include "core/document/object/object.hpp"
#include "core/error.hpp"

namespace ripper::pdf::core
{
    /// Typed view over a PDF pages tree object (/Type /Pages).
    ///
    /// Non-owning wrapper around an `object` stored in the cross-reference table.
    /// Ownership remains with the `cross_reference_entry` that holds the object.
    class pages
    {
    public:
        explicit pages(object &obj) noexcept;

        /// Returns the underlying object.
        [[nodiscard]] object &obj() noexcept;
        [[nodiscard]] const object &obj() const noexcept;

        /// Returns a pointer to the content dictionary, or `nullptr` if content is not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

        /// Returns the total page count from the /Count entry.
        [[nodiscard]] std::expected<std::uint64_t, error> count() const;

    private:
        std::reference_wrapper<object> obj_;
    };
}
