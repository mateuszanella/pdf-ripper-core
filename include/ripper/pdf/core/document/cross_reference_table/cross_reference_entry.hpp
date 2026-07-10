#pragma once

#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"

#include <cstdint>
#include <memory>

namespace ripper::pdf::core
{
class indirect_object;

/// The type of a cross-reference entry (PDF spec §7.5.8).
enum class xref_entry_type : std::uint8_t
{
    free = 0,         ///< Free entry (deleted or never used).
    uncompressed = 1, ///< Uncompressed object at a byte offset in the file.
    compressed = 2    ///< Compressed object stored inside an object stream.
};

/// A single entry in a cross-reference table section.
///
/// Uses flat storage aligned with the PDF xref stream binary format:
/// - `field1_` holds the type-interpreted value (byte offset, object stream number,
///   or next-free-object number).
/// - `field2_` holds the index within an object stream for compressed entries.
///   For uncompressed and free entries, generation is stored in `reference_`.
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
/// Entries are copyable; copying a resolved entry deep-clones the indirect object.
class cross_reference_entry
{
public:
    /// Construct an entry parsed from a traditional cross-reference table.
    ///
    /// The indirect_object is not yet resolved; it will be lazy-loaded on first access
    /// via `cross_reference_table::resolve()`.
    ///
    /// @param ref The indirect reference (object number + generation).
    /// @param offset The byte offset in the file.
    /// @param in_use Whether the entry is in-use (true) or free (false).
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

    /// Construct a compressed (type-2) entry from an xref stream.
    ///
    /// The object is stored inside an object stream at the given index.
    /// The indirect_object is not yet resolved; it will be loaded when the containing
    /// object stream is parsed.
    ///
    /// @param ref The indirect reference (object number + generation).
    /// @param objstm_number The object number of the containing object stream.
    /// @param objstm_index The index of this object within the object stream.
    explicit cross_reference_entry(indirect_reference ref, std::uint32_t objstm_number,
                                   std::uint32_t objstm_index) noexcept;

    /// Copy constructor. Deep-clones the indirect object if resolved.
    cross_reference_entry(const cross_reference_entry& other);

    /// Copy assignment. Deep-clones the indirect object if resolved.
    cross_reference_entry& operator=(const cross_reference_entry& other);

    /// Move constructor.
    cross_reference_entry(cross_reference_entry&& other) noexcept = default;

    /// Move assignment.
    cross_reference_entry& operator=(cross_reference_entry&& other) noexcept = default;

    /// Destructor.
    ~cross_reference_entry() = default;

    /// Returns the indirect reference (object number + generation) for this entry.
    [[nodiscard]] const indirect_reference& reference() const noexcept;

    /// Returns the entry type (free, uncompressed, or compressed).
    [[nodiscard]] xref_entry_type type() const noexcept;

    /// Returns whether this entry is marked as in-use (as opposed to a free entry).
    /// Convenience method equivalent to `type() != xref_entry_type::free`.
    [[nodiscard]] bool in_use() const noexcept;

    /// Returns whether this entry is a compressed (type-2) entry.
    [[nodiscard]] bool is_compressed() const noexcept;

    /// Returns the byte offset within the file where this indirect object resides.
    ///
    /// Valid only for uncompressed (type-1) entries.
    [[nodiscard]] std::uint64_t offset() const noexcept;

    /// Record the byte offset at which this entry's indirect object was written.
    ///
    /// Called during serialization to track the position of newly written objects.
    /// Only meaningful for uncompressed (type-1) entries.
    void set_offset(std::uint64_t offset) noexcept;

    /// Returns the object number of the containing object stream.
    ///
    /// Valid only for compressed (type-2) entries.
    [[nodiscard]] std::uint32_t objstm_number() const noexcept;

    /// Returns the index within the containing object stream.
    ///
    /// Valid only for compressed (type-2) entries.
    [[nodiscard]] std::uint32_t objstm_index() const noexcept;

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
    /// This sets the entry type to free. The actual logic on whether the entry is removed from
    /// the document or kept as a free entry is up to the save logic. On full rewrites the entry
    /// will be pruned from the final object. On incremental updates the entry will be kept but
    /// marked as free.
    void mark_deleted() noexcept;

private:
    indirect_reference reference_;
    xref_entry_type type_ = xref_entry_type::free;

    /// Type-interpreted value.
    /// Type 0 (free):    next free object number.
    /// Type 1 (uncompressed): byte offset in file.
    /// Type 2 (compressed): object number of containing object stream.
    std::uint64_t field1_ = 0;

    /// Index within the containing object stream. Only valid for type 2.
    std::uint32_t field2_ = 0;

    bool is_new_ = false;

    std::unique_ptr<class indirect_object> object_;
};
} // namespace ripper::pdf::core
