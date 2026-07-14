#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/revision.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"

#include <vector>

namespace ripper::pdf::core
{
/// The complete revision history of a PDF document.
///
/// Owns all revisions in chronological order (oldest first, newest last) and
/// composes the cross-reference manager and trailer manager as non-owning
/// compiled views bound to the revisions vector.
///
/// Revision_history is non-movable and non-copyable because the view-managers
/// hold internal references to the revisions vector. It is stored in document
/// via std::unique_ptr.
class revision_history
{
public:
    /// Construct a revision history from a chronologically ordered list of revisions.
    ///
    /// `revisions` must be in oldest-first order. Typically the parser builds this
    /// list by collecting revisions newest-first (following /Prev) and then
    /// reversing before constructing the history.
    explicit revision_history(std::vector<revision> revisions);

    revision_history(const revision_history&) = delete;
    revision_history& operator=(const revision_history&) = delete;
    revision_history(revision_history&&) = delete;
    revision_history& operator=(revision_history&&) = delete;

    /// Returns a mutable reference to the ordered list of all revisions (oldest first).
    [[nodiscard]] std::vector<revision>& revisions() noexcept;

    /// Returns a const reference to the ordered list of all revisions (oldest first).
    [[nodiscard]] const std::vector<revision>& revisions() const noexcept;

    /// Returns a mutable reference to the active (newest) revision.
    [[nodiscard]] revision& active_revision();

    /// Returns a const reference to the active (newest) revision.
    [[nodiscard]] const revision& active_revision() const noexcept;

    /// Append a new revision as the most recent update.
    void push_revision(class revision r);

    /// Returns a mutable reference to the cross-reference manager (compiled view).
    [[nodiscard]] cross_reference_manager& xref() noexcept;

    /// Returns a const reference to the cross-reference manager.
    [[nodiscard]] const cross_reference_manager& xref() const noexcept;

    /// Returns a mutable reference to the trailer manager (compiled view).
    [[nodiscard]] trailer_manager& trailer() noexcept;

    /// Returns a const reference to the trailer manager.
    [[nodiscard]] const trailer_manager& trailer() const noexcept;

    /// Consolidate all revisions into a single revision.
    ///
    /// Computes the newest-wins active entry view across all sections, moves those
    /// entries into a fresh section, compiles the trailer merging oldest-to-newest
    /// and stripping /Prev, and replaces the revision list with a single revision.
    /// Used when performing a full save/rewrite.
    void squash();

    /// Returns the number of revisions in this history.
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<revision> revisions_;
    cross_reference_manager xref_view_;
    class trailer_manager trailer_view_;
};
} // namespace ripper::pdf::core
