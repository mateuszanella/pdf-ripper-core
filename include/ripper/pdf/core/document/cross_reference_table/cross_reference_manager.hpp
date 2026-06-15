#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_table.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace ripper::pdf::core
{
class indirect_object;
class cross_reference_entry;

/// Top-level orchestrator for all cross-reference sections in a PDF document.
///
/// A PDF file may contain multiple cross-reference sections, one per incremental update.
/// The manager owns all of them in chronological order (oldest first, newest last) and
/// provides a unified entry point for all cross-reference operations.
///
/// ## Lookup semantics
///
/// When looking up an entry by object number, the manager scans sections from newest to
/// oldest. The first match is returned, so the most recent revision of any object always
/// takes precedence over earlier revisions.
///
/// ## Allocation semantics
///
/// New indirect objects are allocated into the active section (the last, newest section).
/// If no sections exist, an empty section is created on demand.
///
/// ## Compiled view
///
/// `entries()` returns a flat pointer map compiled across all sections where newest wins
/// for duplicate object numbers. This is the canonical view of the document's current state.
///
/// Implements the `cross_reference_table` interface, sharing the same API as individual
/// `cross_reference_section` objects for uniform access.
class cross_reference_manager : public cross_reference_table
{
public:
    /// Construct a manager from a list of sections in chronological order (oldest first, newest
    /// last).
    ///
    /// `sections` is typically produced by the document structure parser, which collects
    /// sections from newest to oldest (following `startxref` and `/Prev` links) and then
    /// reverses them before passing them here.
    explicit cross_reference_manager(std::vector<cross_reference_section> sections) noexcept;

    cross_reference_manager(const cross_reference_manager&) = delete;
    cross_reference_manager& operator=(const cross_reference_manager&) = delete;
    cross_reference_manager(cross_reference_manager&&) noexcept = default;
    cross_reference_manager& operator=(cross_reference_manager&&) noexcept = default;

    /// Look up a mutable entry by object number across all sections.
    ///
    /// Scans sections from newest to oldest and returns the first matching entry,
    /// ensuring the most recent revision of an object always takes precedence.
    ///
    /// Returns a raw pointer to the entry (valid for the lifetime of this manager),
    /// or `nullptr` if no entry exists for the given object number in any section.
    [[nodiscard]] cross_reference_entry* find(std::uint32_t object_number) noexcept override;

    /// Look up a read-only entry by object number across all sections.
    ///
    /// Scans sections from newest to oldest and returns the first matching entry,
    /// ensuring the most recent revision of an object always takes precedence.
    ///
    /// Returns a raw pointer to the entry (valid for the lifetime of this manager),
    /// or `nullptr` if no entry exists for the given object number in any section.
    [[nodiscard]] const cross_reference_entry*
    find(std::uint32_t object_number) const noexcept override;

    /// Look up a mutable entry by indirect reference across all sections.
    ///
    /// Equivalent to `find(ref.object_number())`. The generation number in `ref`
    /// is not used for the lookup.
    ///
    /// Returns a raw pointer to the entry, or `nullptr` if not found.
    [[nodiscard]] cross_reference_entry* find(const indirect_reference& ref) noexcept override;

    /// Look up a read-only entry by indirect reference across all sections.
    ///
    /// Equivalent to `find(ref.object_number())`. The generation number in `ref`
    /// is not used for the lookup.
    ///
    /// Returns a raw pointer to the entry, or `nullptr` if not found.
    [[nodiscard]] const cross_reference_entry*
    find(const indirect_reference& ref) const noexcept override;

    /// Reserve a slot for a new indirect object and return its assigned indirect reference.
    ///
    /// Computes the next available object number across all sections, creates a pending
    /// entry in the active (newest) section, and returns the assigned indirect reference.
    /// The entry must be committed via `commit()` before it is usable.
    ///
    /// Use `allocate()` for the simpler single-step case.
    [[nodiscard]] indirect_reference reserve() noexcept override;

    /// Commit a constructed indirect object to a previously reserved reference.
    ///
    /// Locates the entry for `ref` across all sections (scanning newest to oldest),
    /// transfers ownership of `object` into it, and marks it as resolved. Returns a
    /// raw non-owning pointer to the committed indirect object.
    ///
    /// Returns `nullptr` if `ref` is not found in any section, or if the entry was not
    /// in a reserved (new and unresolved) state.
    [[nodiscard]] class indirect_object*
    commit(const indirect_reference& ref,
           std::unique_ptr<class indirect_object> object) noexcept override;

    /// Allocate and immediately commit a new in-memory indirect object.
    ///
    /// Combines `reserve()` and `commit()` into a single step. Assigns the next globally
    /// available object number, adds the resolved entry to the active (newest) section,
    /// and returns the assigned indirect reference.
    [[nodiscard]] indirect_reference
    allocate(std::unique_ptr<class indirect_object> object) noexcept override;

    /// Return a compiled flat non-owning pointer map of all entries across all sections.
    ///
    /// Iterates sections from oldest to newest, applying `insert_or_assign` so that newer
    /// entries overwrite older ones for the same object number. The result is the canonical
    /// compiled view of the document: for each object number, the newest revision wins.
    ///
    /// The returned map contains raw pointers into the underlying section storage. Pointers
    /// are valid as long as this manager is not modified.
    [[nodiscard]] std::map<std::uint32_t, cross_reference_entry*> entries() noexcept override;

    /// Return a compiled flat non-owning pointer map of the active (live) entries.
    ///
    /// Builds the same newest-wins merged view as `entries()`, then filters out any entry
    /// where `in_use()` is false, with the single exception of object 0, which is always
    /// retained as the mandatory free-list head.
    ///
    /// The returned map contains raw pointers into the underlying section storage. Pointers
    /// are valid as long as this manager is not modified.
    [[nodiscard]] std::map<std::uint32_t, cross_reference_entry*> active_entries() noexcept;

    /// Returns the total number of entries across all sections.
    ///
    /// Counts every entry in every section, including entries for the same object number
    /// that appear in multiple revisions. To count unique objects, use `entries().size()`.
    [[nodiscard]] std::size_t size() const noexcept override;

    /// Returns the next available object number for allocation across all sections.
    ///
    /// This is the maximum of `next_object_number()` over all sections, ensuring that
    /// newly allocated objects do not collide with any object from any revision.
    /// Returns 1 if no sections or entries are present.
    [[nodiscard]] std::uint32_t next_object_number() const noexcept override;

    /// Returns a mutable reference to the active (last/newest) section for new object allocation.
    ///
    /// If no sections exist, an empty section is created and appended first.
    [[nodiscard]] cross_reference_section& active_section() noexcept;

    /// Returns a read-only view of all sections in chronological order (oldest first, newest last).
    ///
    /// The returned reference is valid for the lifetime of this manager.
    [[nodiscard]] const std::vector<cross_reference_section>& sections() const noexcept;

    /// Returns a mutable view of all sections in chronological order (oldest first, newest last).
    ///
    /// The returned reference is valid for the lifetime of this manager.
    [[nodiscard]] std::vector<cross_reference_section>& sections() noexcept;

    /// Consolidate all sections into a single section containing only the active entries.
    ///
    /// Computes the newest-wins active view across all sections (same logic as
    /// `active_entries()`), moves those entries in object-number order into a fresh
    /// `cross_reference_section`, and replaces the internal section list with that
    /// single section. Superseded revisions and free entries (except object 0) are
    /// discarded.
    ///
    /// After a squash the manager behaves as if the document were freshly created
    /// with exactly the current live objects. Used when running a full save/rewrite.
    void squash() noexcept;

private:
    std::vector<cross_reference_section> sections_;
};
} // namespace ripper::pdf::core
