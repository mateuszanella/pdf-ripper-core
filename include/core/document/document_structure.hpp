#pragma once

#include <vector>

#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/trailer/trailer.hpp"

namespace ripper::pdf::core
{
    /// The assembled PDF document structure.
    ///
    /// Composed of the cross-reference manager (which owns all xref sections) and the
    /// merged trailer dictionary, plus the full trailer history as parsed from the
    /// xref/trailer chain in a PDF file.
    class document_structure
    {
    public:
        /// Construct a document_structure from fully assembled components.
        explicit document_structure(
            cross_reference_manager xref_manager,
            trailer compiled_trailer,
            std::vector<trailer> trailer_history) noexcept;

        /// Returns a reference to the cross-reference manager.
        [[nodiscard]] cross_reference_manager &xref() noexcept;

        /// Returns a const reference to the cross-reference manager.
        [[nodiscard]] const cross_reference_manager &xref() const noexcept;

        /// Returns a reference to the compiled (merged) trailer dictionary.
        [[nodiscard]] trailer &trailer() noexcept;

        /// Returns a const reference to the compiled (merged) trailer dictionary.
        [[nodiscard]] const class trailer &trailer() const noexcept;

        /// Returns a reference to the full trailer traversal history (newest-first order).
        [[nodiscard]] std::vector<class trailer> &trailer_history() noexcept;

        /// Returns a const reference to the full trailer traversal history.
        [[nodiscard]] const std::vector<class trailer> &trailer_history() const noexcept;

    private:
        cross_reference_manager xref_manager_;
        class trailer compiled_trailer_;
        std::vector<class trailer> trailer_history_;
    };
}
