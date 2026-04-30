#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>

#include "core/document/cross_reference_table/cross_reference_entry.hpp"
#include "core/document/object/indirect_reference.hpp"

namespace ripper::core
{
    class object;

    /// Mutable cross-reference table and central object registry for a PDF document.
    ///
    /// Serves as the single source of truth for all indirect objects in the document,
    /// whether parsed from an existing file (lazy-loaded on demand) or newly created
    /// in memory.
    ///
    /// ## Object lifecycle
    ///
    /// Objects from an existing file start as unresolved entries with a known byte offset.
    /// They are materialized on first access via `resolve()`, which invokes a caller-supplied
    /// `loader_fn` to parse the object from disk and cache it inside the entry.
    ///
    /// New objects are introduced either via `allocate()` (single-step) or via the
    /// `reserve()` / `commit()` pair when the object's own constructor requires a known
    /// `indirect_reference` before the entry can exist.
    ///
    /// ## Ownership
    ///
    /// Each `cross_reference_entry` owns its resolved `object` exclusively.
    /// The table is non-copyable.
    class cross_reference_table
    {
    public:
        /// Type alias for the internal mapping of object numbers to cross-reference entries.
        using entry_map = std::map<std::uint32_t, cross_reference_entry>;

        /// Type alias for the loader function used to load and parse an object from file on first access.
        ///
        /// Receives the unresolved entry (which carries the byte offset) and must return
        /// a heap-allocated `object`, or `nullptr` on failure.
        using loader_fn = std::function<std::unique_ptr<class object>(const cross_reference_entry &)>;

        /// Construct a cross-reference table from a pre-built entry map.
        ///
        /// Typically called by the parser after reading the xref section from disk.
        explicit cross_reference_table(entry_map entries) noexcept;

        cross_reference_table(const cross_reference_table &) = delete;
        cross_reference_table &operator=(const cross_reference_table &) = delete;
        cross_reference_table(cross_reference_table &&) noexcept = default;
        cross_reference_table &operator=(cross_reference_table &&) noexcept = default;

        /// Returns a read-only view of all entries in the table.
        [[nodiscard]] const entry_map &entries() const noexcept;

        /// Returns a mutable view of all entries in the table.
        [[nodiscard]] entry_map &entries() noexcept;

        /// Look up an entry by object number.
        ///
        /// Returns a pointer into the table (valid for the lifetime of the table),
        /// or `nullptr` if no entry exists for the given object number.
        [[nodiscard]] cross_reference_entry *find(std::uint32_t object_number) const noexcept;

        /// Look up an entry by indirect reference.
        ///
        /// Equivalent to `find(ref.object_number())`.
        [[nodiscard]] cross_reference_entry *find(const indirect_reference &ref) const noexcept;

        /// Resolve an indirect reference to its in-memory object.
        ///
        /// If the object is already cached in the entry, it is returned immediately.
        /// Otherwise, `loader` is invoked to parse the object from disk and cache it.
        ///
        /// Returns `nullptr` if the entry does not exist or the loader fails.
        [[nodiscard]] class object *resolve(const indirect_reference &ref, const loader_fn &loader) const;

        /// Reserve a slot for a new object, returning its assigned indirect reference.
        ///
        /// The entry is created in a pending state (no object, no offset).
        /// You must call `commit()` with the constructed object before the entry is usable.
        ///
        /// Use this when the object's constructor requires a known `indirect_reference`.
        [[nodiscard]] indirect_reference reserve() noexcept;

        /// Commit a constructed object to a previously reserved reference.
        ///
        /// Transfers ownership of the object to the entry and returns a raw pointer to it.
        ///
        /// Returns `nullptr` if the reference does not correspond to a reserved entry.
        [[nodiscard]] class object *commit(const indirect_reference &ref, std::unique_ptr<class object> object) noexcept;

        /// Allocate and immediately commit a new in-memory object.
        ///
        /// Use this when the object can be constructed without knowing its reference
        /// in advance (e.g. the reference is injected after construction).
        ///
        /// Returns the assigned indirect reference.
        [[nodiscard]] indirect_reference allocate(std::unique_ptr<class object> object) noexcept;

        /// Returns the number of entries currently in the table.
        [[nodiscard]] std::size_t size() const noexcept;

        /// Returns the next available object number.
        ///
        /// Object numbers are assigned sequentially. This is used internally by
        /// `reserve()` and `allocate()`, but exposed for introspection.
        [[nodiscard]] std::uint32_t next_object_number() const noexcept;

    private:
        entry_map entries_;
    };
}
