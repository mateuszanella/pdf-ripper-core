#pragma once

#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace ripper::pdf::core
{
class indirect_object;

/// A single entry in a cross-reference table.
///
/// An entry can be in one of four meaningful states:
///
///   - **On disk, not resolved**: parsed from file, indirect_object not yet loaded into memory.
///     `is_new() == false`, `is_resolved() == false`
///
///   - **On disk, resolved**: parsed from file and lazy-loaded into memory.
///     `is_new() == false`, `is_resolved() == true`
///
///   - **In memory, new**: constructed programmatically, no file backing yet.
///     `is_new() == true`, `is_resolved() == true`
///
///   - **Reserved, pending**: slot reserved via `cross_reference_table::reserve()`,
///     awaiting `commit()`. `is_new() == true`, `is_resolved() == false`
///
/// Entries are non-copyable due to unique ownership of the resolved indirect object.
class cross_reference_entry
{
public:
    /// Construct an entry parsed from a traditional cross-reference table.
    ///
    /// The indirect_object is not yet resolved; it will be lazy-loaded on first access
    /// via `cross_reference_table::resolve()`.
    explicit cross_reference_entry(indirect_reference ref, std::uint64_t offset,
                                   bool in_use) noexcept;

    /// Construct a pending reserved entry with no file backing and no indirect_object yet.
    ///
    /// Used internally by `cross_reference_table::reserve()`.
    /// Must be committed via `cross_reference_table::commit()` before use.
    explicit cross_reference_entry(indirect_reference ref) noexcept;

    /// Construct a new in-memory entry with no file backing.
    ///
    /// Used when creating new objects programmatically.
    /// The indirect object is immediately available; no file offset will ever exist
    /// until the document is saved.
    explicit cross_reference_entry(indirect_reference ref,
                                   std::unique_ptr<indirect_object> indirect_object) noexcept;

    /// Returns the indirect reference (object number + generation) for this entry.
    [[nodiscard]] const indirect_reference& reference() const noexcept;

    /// Returns the byte offset within the file where this indirect object resides.
    ///
    /// Returns `std::nullopt` for new in-memory objects that have not yet been written.
    [[nodiscard]] const std::optional<std::uint64_t>& offset() const noexcept;

    /// Record the byte offset at which this entry's indirect object was written.
    ///
    /// Called during serialization to track the position of newly written objects.
    void set_offset(std::uint64_t offset) noexcept;

    /// Returns whether this entry is marked as in-use (as opposed to a free entry).
    [[nodiscard]] bool in_use() const noexcept;

    /// Returns whether the indirect object has been loaded or constructed in memory.
    [[nodiscard]] bool is_resolved() const noexcept;

    /// Returns whether this entry has no reader backing (i.e. was created in memory).
    [[nodiscard]] bool is_new() const noexcept;

    /// Returns a raw pointer to the resolved indirect object, or `nullptr` if not yet resolved.
    ///
    /// Ownership remains with this entry.
    [[nodiscard]] class indirect_object* indirect_object() const noexcept;

    /// Resolves this entry by caching an indirect object into it.
    ///
    /// Intended for lazy-loading: called by the resolver after parsing the indirect object
    /// from disk. Also used when committing newly created objects via `commit()`.
    ///
    /// If this entry was already resolved, the previous indirect object is replaced.
    /// Any existing raw pointer to the old object is invalidated.
    ///
    /// Returns a raw pointer to the newly cached indirect object, or `nullptr` if `obj` is null.
    [[nodiscard]] class indirect_object*
    resolve(std::unique_ptr<class indirect_object> obj) noexcept;

    /// Marks this entry as deleted.
    ///
    /// This sets the in-use flag to false. The actual logic on wether the entry is removed from
    /// the document or kept as a free entry is up to the save logic. On full rewrites the entry
    /// will be pruned from the final object. On incremental updates the entry will be kept but
    /// marked as free.
    void mark_deleted() noexcept;

private:
    indirect_reference reference_;
    std::optional<std::uint64_t> offset_;
    bool in_use_;

    std::unique_ptr<class indirect_object> object_;
};
} // namespace ripper::pdf::core
