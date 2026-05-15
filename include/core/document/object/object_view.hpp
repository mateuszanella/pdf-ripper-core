#pragma once

#include <functional>

#include "core/document/object/indirect_object.hpp"

namespace ripper::pdf::core
{
    /// Non-owning, type-erased view over a PDF indirect_object.
    /// Provides common forwarding helpers for all typed wrappers.
    class object_view
    {
    public:
        explicit object_view(indirect_object &obj) noexcept;

        virtual ~object_view() = default;

        /// Returns a reference to the underlying indirect_object.
        [[nodiscard]] indirect_object &obj() noexcept;

        /// Returns a const reference to the underlying indirect_object.
        [[nodiscard]] const indirect_object &obj() const noexcept;

        /// Returns a pointer to the content dictionary, or nullptr if not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;

        /// Returns a const pointer to the content dictionary, or nullptr if not a dictionary.
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

    protected:
        std::reference_wrapper<indirect_object> obj_;
    };
}
