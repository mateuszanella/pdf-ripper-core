#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"

namespace ripper::pdf::core
{
/// A single PDF revision: one cross-reference section paired with its trailer.
///
/// A PDF file with incremental updates contains one revision per update. Each
/// revision carries the xref section that records the objects written in that
/// update and the trailer dictionary that provides document-level metadata
/// (Root, Size, Prev, ID, etc.).
///
/// Objects belonging to this revision live inside the cross-reference section's
/// entries (via cross_reference_entry::indirect_object); no separate object
/// container exists on this class.
class revision
{
public:
    /// Construct a revision from a section and its matching trailer.
    explicit revision(cross_reference_section section, class trailer t) noexcept;

    /// Returns a mutable reference to the cross-reference section.
    [[nodiscard]] cross_reference_section& section() noexcept;

    /// Returns a const reference to the cross-reference section.
    [[nodiscard]] const cross_reference_section& section() const noexcept;

    /// Returns a mutable reference to the trailer dictionary.
    [[nodiscard]] class trailer& trailer() noexcept;

    /// Returns a const reference to the trailer dictionary.
    [[nodiscard]] const class trailer& trailer() const noexcept;

private:
    cross_reference_section section_;
    class trailer trailer_;
};
} // namespace ripper::pdf::core
