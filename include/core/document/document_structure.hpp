#pragma once

#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"

namespace ripper::core
{
    /// The assembled PDF document structure.
    ///
    /// Composed of the merged cross-reference table, the merged trailer dictionary
    /// and the full history of each as parsed from the xref/trailer chain in a PDF file.
    class document_structure
    {
    public:
        /// Construct a document_structure from fully assembled components.
        explicit document_structure(
            cross_reference_table compiled_xref,
            std::vector<cross_reference_table> xref_history,
            trailer compiled_trailer,
            std::vector<trailer> trailer_history) noexcept;

        /// Returns a reference to the compiled (merged) cross-reference table.
        [[nodiscard]] cross_reference_table &xref() noexcept;

        /// Returns a const reference to the compiled (merged) cross-reference table.
        [[nodiscard]] const cross_reference_table &xref() const noexcept;

        /// Returns a reference to the full xref traversal history (newest-first order).
        [[nodiscard]] std::vector<cross_reference_table> &xref_history() noexcept;

        /// Returns a const reference to the full xref traversal history.
        [[nodiscard]] const std::vector<cross_reference_table> &xref_history() const noexcept;

        /// Returns a reference to the compiled (merged) trailer dictionary.
        [[nodiscard]] trailer &trailer() noexcept;

        /// Returns a const reference to the compiled (merged) trailer dictionary.
        [[nodiscard]] const class trailer &trailer() const noexcept;

        /// Returns a reference to the full trailer traversal history (newest-first order).
        [[nodiscard]] std::vector<class trailer> &trailer_history() noexcept;

        /// Returns a const reference to the full trailer traversal history.
        [[nodiscard]] const std::vector<class trailer> &trailer_history() const noexcept;

    private:
        cross_reference_table compiled_xref_;
        std::vector<cross_reference_table> xref_history_;
        class trailer compiled_trailer_;
        std::vector<class trailer> trailer_history_;
    };
}
