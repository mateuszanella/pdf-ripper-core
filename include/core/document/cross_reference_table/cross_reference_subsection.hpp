#pragma once

#include <cstdint>
#include <map>

#include "core/document/cross_reference_table/cross_reference_entry.hpp"
#include "core/document/object/indirect_reference.hpp"

namespace ripper::pdf::core
{
    /// A single contiguous subsection within a PDF cross-reference section.
    ///
    /// Per the PDF spec (§7.5.4), each subsection is introduced by a header line:
    ///   `<first_object_number> <count>`
    /// followed by exactly `count` 20-byte entries, one per object in the range.
    ///
    /// Every entry has a consecutive object number beginning at `first_object_number()`, so
    /// the range covered is [`first_object_number`, `first_object_number + count - 1`].
    ///
    /// A subsection is non-copyable due to unique ownership of each `cross_reference_entry`.
    class cross_reference_subsection
    {
    public:
        /// Type alias for the owning map of object numbers to their cross-reference entries.
        ///
        /// Entries in this map always have consecutive keys starting from `first_object_number()`.
        using entry_map = std::map<std::uint32_t, cross_reference_entry>;

        /// Construct a subsection with the given first object number and its initial entries.
        ///
        /// `entries` should contain only object numbers starting at `first_object_number` and
        /// increasing consecutively; this invariant is not validated at construction time.
        explicit cross_reference_subsection(std::uint32_t first_object_number, entry_map entries) noexcept;

        cross_reference_subsection(const cross_reference_subsection &) = delete;
        cross_reference_subsection &operator=(const cross_reference_subsection &) = delete;
        cross_reference_subsection(cross_reference_subsection &&) noexcept = default;
        cross_reference_subsection &operator=(cross_reference_subsection &&) noexcept = default;

        /// Returns the object number of the first entry in this subsection.
        ///
        /// Together with `count()`, defines the object number range covered:
        /// [`first_object_number()`, `first_object_number() + count() - 1`].
        [[nodiscard]] std::uint32_t first_object_number() const noexcept;

        /// Returns the number of entries in this subsection.
        ///
        /// Equivalent to `entries().size()`. Corresponds to the count value on the subsection
        /// header line written during serialization.
        [[nodiscard]] std::uint32_t count() const noexcept;

        /// Returns a read-only view of all entries in this subsection, keyed by object number.
        ///
        /// Iterating this map in order yields entries with consecutive object numbers starting
        /// at `first_object_number()`. The returned reference is valid for the lifetime of
        /// this subsection.
        [[nodiscard]] const entry_map &entries() const noexcept;

        /// Returns a mutable view of all entries in this subsection, keyed by object number.
        ///
        /// Iterating this map in order yields entries with consecutive object numbers starting
        /// at `first_object_number()`. The returned reference is valid for the lifetime of
        /// this subsection.
        [[nodiscard]] entry_map &entries() noexcept;

        /// Look up a mutable entry by object number.
        ///
        /// Returns a raw pointer to the matching entry (valid for the lifetime of this
        /// subsection), or `nullptr` if `object_number` is not within this subsection's range.
        [[nodiscard]] cross_reference_entry *find(std::uint32_t object_number) noexcept;

        /// Look up a read-only entry by object number.
        ///
        /// Returns a raw pointer to the matching entry (valid for the lifetime of this
        /// subsection), or `nullptr` if `object_number` is not within this subsection's range.
        [[nodiscard]] const cross_reference_entry *find(std::uint32_t object_number) const noexcept;

    private:
        std::uint32_t first_object_number_;
        entry_map entries_;
    };
}
