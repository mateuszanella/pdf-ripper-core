#pragma once

#include "ripper/pdf/core/document/object/indirect_object.hpp"

#include <functional>

namespace ripper::pdf::core
{
/// Non-owning, type-erased view over a PDF indirect_object.
/// Provides common forwarding helpers for all typed wrappers.
class object_view
{
public:
    explicit object_view(indirect_object& obj) noexcept;

    /// Returns a reference to the underlying indirect_object.
    [[nodiscard]] indirect_object& obj() noexcept;

    /// Returns a const reference to the underlying indirect_object.
    [[nodiscard]] const indirect_object& obj() const noexcept;

    /// Returns a pointer to the content dictionary, or nullptr if not a dictionary.
    [[nodiscard]] class dictionary* dictionary() noexcept;

    /// Returns a const pointer to the content dictionary, or nullptr if not a dictionary.
    [[nodiscard]] const class dictionary* dictionary() const noexcept;

    /// Rebind this view to a different indirect_object.
    ///
    /// After this call, all operations on this view target `obj`.
    void rebind(indirect_object& obj) noexcept;

    /// Rebind this view to the object's clone in the active (newest) xref section.
    ///
    /// After this call, all modifications through this view affect the active revision
    /// and will be captured during incremental save.
    ///
    /// If the object is already in the active section, this is a no-op.
    ///
    /// @throws logic_exception if the object cannot be found or cloned.
    void rebind_to_active_revision();

protected:
    std::reference_wrapper<indirect_object> obj_;
};
} // namespace ripper::pdf::core
