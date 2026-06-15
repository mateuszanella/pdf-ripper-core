#pragma once

#include "core/document/trailer/trailer.hpp"

#include <vector>

namespace ripper::pdf::core
{
/// Owns and manages the complete trailer history of a PDF document.
///
/// A PDF document with incremental updates contains one trailer per revision,
/// each pointing to the previous one via the `/Prev` byte offset. The
/// `trailer_manager` stores all trailers in chronological order (oldest first,
/// newest last) and provides a compiled (merged) view that represents the
/// effective trailer dictionary for the document as a whole.
///
/// ## Ordering
///
/// Trailers are stored oldest-first (index 0 is the original trailer from the
/// first revision, `back()` is the most recently added revision). This mirrors
/// the ordering convention used by `cross_reference_manager`.
///
/// ## Compiled view
///
/// `compiled()` iterates oldest-to-newest and merges all dictionaries so that
/// newer values overwrite older ones. This matches the PDF specification rule
/// that the trailer chain must be followed from the most recent trailer back
/// to the first, with later values taking precedence.
///
/// ## Active trailer
///
/// `active_trailer()` returns a reference to the most recently added trailer
/// (the `back()` of the internal vector). Writers that produce a new revision
/// modify the active trailer. An empty trailer is created on demand if none
/// exist yet.
///
class trailer_manager
{
public:
    /// Construct a trailer_manager from a chronologically ordered list of trailers.
    ///
    /// `trailers` must be in oldest-first order. Typically the parser builds this
    /// list by collecting trailers newest-first (following `/Prev`) and then
    /// reversing before constructing the manager.
    explicit trailer_manager(std::vector<trailer> trailers) noexcept;

    /// Returns a mutable reference to the ordered list of all trailers (oldest first).
    ///
    /// Primarily useful for serialisation and diagnostic tooling that need to
    /// enumerate every revision trailer individually.
    [[nodiscard]] std::vector<trailer>& trailers() noexcept;

    /// Returns a const reference to the ordered list of all trailers (oldest first).
    [[nodiscard]] const std::vector<trailer>& trailers() const noexcept;

    /// Returns a mutable reference to the active (newest) trailer.
    ///
    /// The active trailer is the one modified by writers when producing a new
    /// revision. It is always the last element of the internal vector. If no
    /// trailers exist yet, an empty trailer backed by an empty dictionary is
    /// created and pushed before returning.
    [[nodiscard]] trailer& active_trailer() noexcept;

    /// Returns a const reference to the active (newest) trailer.
    ///
    /// Requires that at least one trailer has been added. Behaviour is undefined
    /// if the manager is empty — ensure `push()` or the constructor has populated
    /// the manager before calling the const overload.
    [[nodiscard]] const trailer& active_trailer() const noexcept;

    /// Push a new trailer as the most recent revision.
    ///
    /// The pushed trailer becomes the new active trailer and will be included in
    /// subsequent `compiled()` results.
    void push(trailer t) noexcept;

    /// Returns the compiled (merged) trailer dictionary.
    ///
    /// Iterates all stored trailers oldest-to-newest and merges their dictionary
    /// entries so that newer values overwrite older ones. The result is a new
    /// `trailer` object computed on each call; it is not cached.
    ///
    /// Returns an empty-dictionary trailer if no trailers have been added.
    [[nodiscard]] trailer compiled() const;

    /// Returns the number of trailers stored in this manager.
    [[nodiscard]] std::size_t size() const noexcept;

    /// Consolidate all trailers into a single trailer.
    ///
    /// Computes the same newest-wins merged view as `compiled()`, strips the `/Prev`
    /// entry (which is meaningless after consolidation), and replaces the internal
    /// trailer list with the resulting single trailer. Used when performing a full
    /// save/rewrite where the incremental revision history is discarded.
    void squash() noexcept;

private:
    std::vector<trailer> trailers_;
};
} // namespace ripper::pdf::core
