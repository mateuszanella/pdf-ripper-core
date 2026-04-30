#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "core/document/object/indirect_reference.hpp"

namespace ripper::core
{
    class object;

    /// A single entry in a cross-reference table.
    ///
    /// An entry can be in one of four meaningful states:
    ///
    ///   - **On disk, not resolved**: parsed from file, object not yet loaded into memory.
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
    /// Entries are non-copyable due to unique ownership of the resolved object.
    class cross_reference_entry
    {
    public:
        /// Construct an entry parsed from a traditional cross-reference table.
        ///
        /// The object is not yet resolved; it will be lazy-loaded on first access
        /// via `cross_reference_table::resolve()`.
        cross_reference_entry(indirect_reference ref, std::uint64_t offset, bool in_use) noexcept;

        /// Construct a pending reserved entry with no file backing and no object yet.
        ///
        /// Used internally by `cross_reference_table::reserve()`.
        /// Must be committed via `cross_reference_table::commit()` before use.
        explicit cross_reference_entry(indirect_reference ref) noexcept;

        /// Construct a new in-memory entry with no file backing.
        ///
        /// Used when creating new objects programmatically.
        /// The object is immediately available; no file offset will ever exist
        /// until the document is saved.
        cross_reference_entry(indirect_reference ref, std::unique_ptr<object> object) noexcept;

        /// Returns the indirect reference (object number + generation) for this entry.
        [[nodiscard]] const indirect_reference &reference() const noexcept;

        /// Returns the byte offset within the file where this object resides.
        ///
        /// Returns `std::nullopt` for new in-memory objects that have not yet been written.
        [[nodiscard]] const std::optional<std::uint64_t> &offset() const noexcept;

        /// Returns whether this entry is marked as in-use (as opposed to a free entry).
        [[nodiscard]] bool in_use() const noexcept;

        /// Returns whether the object has been loaded or constructed in memory.
        [[nodiscard]] bool is_resolved() const noexcept;

        /// Returns whether this entry has no reader backing (i.e. was created in memory).
        [[nodiscard]] bool is_new() const noexcept;

        /// Returns a raw pointer to the resolved object, or `nullptr` if not yet resolved.
        ///
        /// Ownership remains with this entry.
        [[nodiscard]] class object *object() const noexcept;

        /// Caches a resolved object into this entry.
        ///
        /// Intended for lazy-loading: called by the resolver after parsing the object
        /// from disk. Replaces any previously cached object.
        ///
        /// Returns a raw pointer to the resolved object, or `nullptr` if the reference
        /// does not correspond to a reserved entry.
        [[nodiscard]] class object *resolve(std::unique_ptr<class object> object) noexcept;

    private:
        indirect_reference reference_;
        std::optional<std::uint64_t> offset_;
        bool in_use_;

        mutable std::unique_ptr<class object> object_;
    };
}
