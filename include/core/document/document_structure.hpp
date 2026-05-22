#pragma once

#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/trailer/trailer_manager.hpp"

namespace ripper::pdf::core
{
    /// The assembled PDF document structure.
    ///
    /// Composed of the cross-reference manager (which owns all xref sections) and the
    /// trailer manager (which owns the complete trailer history for all revisions).
    class document_structure
    {
    public:
        /// Construct a document_structure from fully assembled components.
        explicit document_structure(
            cross_reference_manager xref_manager,
            trailer_manager trailer_manager) noexcept;

        /// Returns a mutable reference to the cross-reference manager.
        [[nodiscard]] cross_reference_manager &xref() noexcept;

        /// Returns a const reference to the cross-reference manager.
        [[nodiscard]] const cross_reference_manager &xref() const noexcept;

        /// Returns a mutable reference to the trailer manager.
        [[nodiscard]] trailer_manager &trailer() noexcept;

        /// Returns a const reference to the trailer manager.
        [[nodiscard]] const trailer_manager &trailer() const noexcept;

    private:
        cross_reference_manager xref_manager_;
        class trailer_manager trailer_manager_;
    };
}
