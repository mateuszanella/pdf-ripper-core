#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/dictionary.hpp"

namespace ripper::core
{
    /// Represents a PDF name object (e.g. `/Type`, `/Pages`).
    ///
    /// Distinct from `std::string` to preserve the semantic difference between
    /// PDF name objects and PDF string objects at the type level.
    struct name
    {
        std::string value;
    };

    /// Represents the PDF null object.
    struct null
    {
    };

    /// Type alias for a PDF array representing an ordered sequence of `value` objects.
    using array = std::vector<value>;

    /// A discriminated union representing any PDF direct object.
    ///
    /// Covers all primitive and composite PDF value types as defined in the PDF spec:
    ///   - Null (`null`)
    ///   - Boolean (`bool`)
    ///   - Integer (`std::int64_t`)
    ///   - Real (`double`)
    ///   - String (`std::string`)
    ///   - Name (`name`)
    ///   - Array (`array`)
    ///   - Dictionary (`dictionary`)
    ///   - Indirect Reference (`indirect_reference`)
    ///
    /// ## Type safety
    ///
    /// `name` and `std::string` are kept as distinct alternatives to prevent
    /// accidental mixing of PDF name objects and PDF string objects, which have
    /// different syntax and semantics in the spec.
    ///
    /// ## Access patterns
    ///
    /// Type-check helpers (`is_*`) and typed accessors (`as_*`) are provided for
    /// ergonomic use in parsing and traversal code. When performance or exhaustive
    /// matching is needed, `variant()` exposes the raw `std::variant` for use with
    /// `std::visit`.
    class value
    {
    public:
        /// The underlying variant type holding all possible PDF value alternatives.
        using variant_type = std::variant<
            null,               ///> null
            bool,               ///> boolean
            std::int64_t,       ///> integer
            double,             ///> real
            std::string,        ///> string
            name,               ///> name
            indirect_reference, ///> reference
            array,              ///> array
            dictionary          ///> dictionary
            >;

        /// Construct a null PDF value.
        value() noexcept;

        /// Construct a boolean PDF value.
        explicit value(bool value) noexcept;

        /// Construct an integer PDF value.
        explicit value(std::int64_t value) noexcept;

        /// Construct a real PDF value.
        explicit value(double value) noexcept;

        /// Construct a string PDF value.
        explicit value(std::string value) noexcept;

        /// Construct a name PDF value.
        explicit value(name value) noexcept;

        /// Construct an array PDF value.
        explicit value(array value) noexcept;

        /// Construct a dictionary PDF value.
        explicit value(dictionary value) noexcept;

        /// Construct an indirect reference PDF value.
        explicit value(indirect_reference value) noexcept;

        /// Returns `true` if this value holds a PDF null object.
        [[nodiscard]] bool is_null() const noexcept;

        /// Returns `true` if this value holds a PDF boolean object.
        [[nodiscard]] bool is_bool() const noexcept;

        /// Returns `true` if this value holds a PDF integer object.
        [[nodiscard]] bool is_integer() const noexcept;

        /// Returns `true` if this value holds a PDF real object.
        [[nodiscard]] bool is_real() const noexcept;

        /// Returns `true` if this value holds a PDF string object.
        [[nodiscard]] bool is_string() const noexcept;

        /// Returns `true` if this value holds a PDF name object.
        [[nodiscard]] bool is_name() const noexcept;

        /// Returns `true` if this value holds a PDF array object.
        [[nodiscard]] bool is_array() const noexcept;

        /// Returns `true` if this value holds a PDF dictionary object.
        [[nodiscard]] bool is_dictionary() const noexcept;

        /// Returns `true` if this value holds a PDF indirect reference.
        [[nodiscard]] bool is_indirect_reference() const noexcept;

        /// Returns a pointer to the held boolean, or `nullptr` if this is not a boolean value.
        [[nodiscard]] const bool *as_bool() const noexcept;

        /// Returns a pointer to the held integer, or `nullptr` if this is not an integer value.
        [[nodiscard]] const std::int64_t *as_integer() const noexcept;

        /// Returns a pointer to the held real, or `nullptr` if this is not a real value.
        [[nodiscard]] const double *as_real() const noexcept;

        /// Returns a pointer to the held string, or `nullptr` if this is not a string value.
        [[nodiscard]] const std::string *as_string() const noexcept;

        /// Returns a pointer to the held name, or `nullptr` if this is not a name value.
        [[nodiscard]] const name *as_name() const noexcept;

        /// Returns a pointer to the held array, or `nullptr` if this is not an array value.
        [[nodiscard]] const array *as_array() const noexcept;

        /// Returns a pointer to the held dictionary, or `nullptr` if this is not a dictionary value.
        [[nodiscard]] const dictionary *as_dictionary() const noexcept;

        /// Returns a pointer to the held indirect reference, or `nullptr` if this is not an indirect reference value.
        [[nodiscard]] const indirect_reference *as_indirect_reference() const noexcept;

        /// Returns the raw underlying variant for use with `std::visit`.
        ///
        /// Prefer the typed `as_*` accessors for single-type access.
        /// Use this when exhaustive matching over all alternatives is needed.
        [[nodiscard]] const variant_type &variant() const noexcept;

    private:
        variant_type value_;
    };
}
