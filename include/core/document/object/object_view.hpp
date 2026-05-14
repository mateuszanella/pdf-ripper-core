#pragma once

#include <functional>

#include "core/document/object/object.hpp"

namespace ripper::pdf::core
{
    /// Non-owning, type-erased view over a PDF object.
    /// Provides common forwarding helpers for all typed wrappers.
    class object_view
    {
    public:
        explicit object_view(object &obj) noexcept;

        virtual ~object_view() = default;

        /// Returns a reference to the underlying object.
        [[nodiscard]] object &obj() noexcept;

        /// Returns a const reference to the underlying object.
        [[nodiscard]] const object &obj() const noexcept;

        /// Returns a pointer to the content dictionary, or nullptr if not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;

        /// Returns a const pointer to the content dictionary, or nullptr if not a dictionary.
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

    protected:
        std::reference_wrapper<object> obj_;
    };
}
