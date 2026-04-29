#pragma once

#include <cstdint>
#include <functional>

namespace ripper::core
{
    /// Represents a PDF indirect object reference.
    ///
    /// In the PDF specification, indirect references identify objects by a unique
    /// combination of an object number and a generation number, written as
    /// `<object_number> <generation> R` in PDF syntax (e.g. `12 0 R`).
    ///
    /// Object numbers are assigned sequentially and uniquely identify an object
    /// within a document. Generation numbers start at 0 and are incremented when
    /// an object is replaced in an incremental update.
    ///
    /// This type is hashable and fully ordered, making it suitable for use as a
    /// key in associative containers such as `std::unordered_map` or `std::map`.
    class indirect_reference
    {
    public:
        /// Construct a null indirect reference.
        ///
        /// Both `object_number` and `generation` are initialized to zero.
        /// A null reference does not correspond to any valid PDF object.
        constexpr indirect_reference() noexcept;

        /// Construct an indirect reference from an object number and generation.
        ///
        /// @param object_number  The unique object number within the PDF document.
        /// @param generation     The generation number; typically 0 for new objects.
        constexpr indirect_reference(std::uint32_t object_number, std::uint16_t generation) noexcept;

        /// Return the object number component of this reference.
        [[nodiscard]] constexpr std::uint32_t object_number() const noexcept;

        /// Return the generation number component of this reference.
        [[nodiscard]] constexpr std::uint16_t generation() const noexcept;

        /// Equality comparison. Two references are equal if both components match.
        constexpr bool operator==(const indirect_reference &other) const noexcept = default;

        /// Three-way comparison. References are ordered first by object number,
        /// then by generation number.
        constexpr auto operator<=>(const indirect_reference &other) const noexcept = default;

    private:
        std::uint32_t object_number_;
        std::uint16_t generation_;
    };
}

/// Specialization of `std::hash` for `ripper::core::indirect_reference`.
///
/// Enables use of `indirect_reference` as a key in `std::unordered_map` and
/// other hash-based containers.
template <>
struct std::hash<ripper::core::indirect_reference>
{
    std::size_t operator()(const ripper::core::indirect_reference &ref) const noexcept;
};
