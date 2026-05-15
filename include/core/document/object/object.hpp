#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/document/object/indirect_reference.hpp"

namespace ripper::pdf::core
{
    /// General header for all PDF object types, including primitives (null, bool, int,
    /// real, string, name) and composites (array, dictionary, indirect reference).

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

    class object;
    class dictionary;

    /// Type alias for a PDF array representing an ordered sequence of `object` objects.
    using array = std::vector<object>;

    /// A discriminated union representing any PDF direct object.
    ///
    /// Covers all primitive and composite PDF object types as defined in the PDF spec:
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
    class object
    {
    public:
        /// The underlying variant type holding all possible PDF object alternatives.
        using variant_type = std::variant<
            null,                       ///> null
            bool,                       ///> boolean
            std::int64_t,               ///> integer
            double,                     ///> real
            std::string,                ///> string
            name,                       ///> name
            indirect_reference,         ///> reference
            array,                      ///> array
            std::unique_ptr<dictionary> ///> dictionary
            >;

        /// Construct a null PDF object.
        object() noexcept;

        /// Construct a boolean PDF object.
        explicit object(bool value) noexcept;

        /// Construct an integer PDF object.
        explicit object(std::int64_t value) noexcept;

        /// Construct a real PDF object.
        explicit object(double value) noexcept;

        /// Construct a string PDF object.
        explicit object(std::string value) noexcept;

        /// Construct a name PDF object.
        explicit object(name value) noexcept;

        /// Construct an array PDF object.
        explicit object(array value) noexcept;

        /// Construct a dictionary PDF object.
        explicit object(dictionary value) noexcept;

        /// Construct an indirect reference PDF object.
        explicit object(indirect_reference value) noexcept;

        ~object() = default;

        object(const object &other);
        object(object &&) noexcept = default;

        object &operator=(const object &other);
        object &operator=(object &&) noexcept = default;

        /// Returns `true` if this object holds a PDF null object.
        [[nodiscard]] bool is_null() const noexcept;

        /// Returns `true` if this object holds a PDF boolean object.
        [[nodiscard]] bool is_bool() const noexcept;

        /// Returns `true` if this object holds a PDF integer object.
        [[nodiscard]] bool is_integer() const noexcept;

        /// Returns `true` if this object holds a PDF real object.
        [[nodiscard]] bool is_real() const noexcept;

        /// Returns `true` if this object holds a PDF string object.
        [[nodiscard]] bool is_string() const noexcept;

        /// Returns `true` if this object holds a PDF name object.
        [[nodiscard]] bool is_name() const noexcept;

        /// Returns `true` if this object holds a PDF array object.
        [[nodiscard]] bool is_array() const noexcept;

        /// Returns `true` if this object holds a PDF dictionary object.
        [[nodiscard]] bool is_dictionary() const noexcept;

        /// Returns `true` if this object holds a PDF indirect reference.
        [[nodiscard]] bool is_indirect_reference() const noexcept;

        /// Returns a pointer to the held boolean, or `nullptr` if this is not a boolean object.
        [[nodiscard]] const bool *as_bool() const noexcept;

        /// Returns a pointer to the held integer, or `nullptr` if this is not an integer object.
        [[nodiscard]] const std::int64_t *as_integer() const noexcept;

        /// Returns a pointer to the held real, or `nullptr` if this is not a real object.
        [[nodiscard]] const double *as_real() const noexcept;

        /// Returns a pointer to the held string, or `nullptr` if this is not a string object.
        [[nodiscard]] const std::string *as_string() const noexcept;

        /// Returns a pointer to the held name, or `nullptr` if this is not a name object.
        [[nodiscard]] const name *as_name() const noexcept;

        /// Returns a pointer to the held array, or `nullptr` if this is not an array object.
        [[nodiscard]] const array *as_array() const noexcept;

        /// Returns a pointer to the held dictionary, or `nullptr` if this is not a dictionary object.
        [[nodiscard]] const dictionary *as_dictionary() const noexcept;

        /// Returns a pointer to the held dictionary, or `nullptr` if this is not a dictionary object.
        [[nodiscard]] dictionary *as_dictionary() noexcept;

        /// Returns a pointer to the held indirect reference, or `nullptr` if this is not an indirect reference object.
        [[nodiscard]] const indirect_reference *as_indirect_reference() const noexcept;

        /// Returns the raw underlying variant for use with `std::visit`.
        ///
        /// Prefer the typed `as_*` accessors for single-type access.
        /// Use this when exhaustive matching over all alternatives is needed.
        [[nodiscard]] const variant_type &variant() const noexcept;

    private:
        variant_type value_;
    };

    /// A PDF dictionary object. An associative map of name keys to `object` objects.
    ///
    /// Wraps the raw `unordered_map<string, object>` representation with typed
    /// accessors and lookup helpers to reduce boilerplate in parsing and traversal code.
    ///
    /// ## Key convention
    ///
    /// Keys are stored without the leading `/` (e.g. `"Type"`, not `"/Type"`).
    /// This matches how parsed PDF names should be normalized before insertion.
    ///
    /// ## Access patterns
    ///
    /// Raw access via `get()` returns a pointer to the held `object`, or `nullptr`
    /// if the key is absent. Typed accessors (`get_name`, `get_integer`, etc.) combine
    /// the lookup and type-check into a single step, returning `nullptr` when either
    /// the key is missing or the value holds a different type.
    ///
    /// When full traversal is needed, `entries()` exposes the underlying map directly.
    class dictionary
    {
    public:
        /// Type alias for the underlying storage.
        using dictionary_map_type = std::unordered_map<std::string, object>;

        /// Construct an empty dictionary.
        dictionary() noexcept = default;

        /// Construct a dictionary from an existing map.
        explicit dictionary(dictionary_map_type entries) noexcept;

        /// Insert or overwrite a key-value pair.
        void set(std::string key, object value);

        /// Remove an entry by key.
        ///
        /// Returns `true` if the key existed and was removed, `false` otherwise.
        bool remove(const std::string &key) noexcept;

        /// Returns `true` if the dictionary contains the given key.
        [[nodiscard]] bool contains(const std::string &key) const noexcept;

        /// Returns the number of entries in the dictionary.
        [[nodiscard]] std::size_t size() const noexcept;

        /// Returns `true` if the dictionary has no entries.
        [[nodiscard]] bool empty() const noexcept;

        /// Returns a pointer to the object for `key`, or `nullptr` if absent.
        [[nodiscard]] const object *get(const std::string &key) const noexcept;

        /// Returns a pointer to the boolean object for `key`,
        /// or `nullptr` if absent or not a boolean.
        [[nodiscard]] const bool *get_bool(const std::string &key) const noexcept;

        /// Returns a pointer to the integer object for `key`,
        /// or `nullptr` if absent or not an integer.
        [[nodiscard]] const std::int64_t *get_integer(const std::string &key) const noexcept;

        /// Returns a pointer to the real object for `key`,
        /// or `nullptr` if absent or not a real.
        [[nodiscard]] const double *get_real(const std::string &key) const noexcept;

        /// Returns a pointer to the string object for `key`,
        /// or `nullptr` if absent or not a string.
        [[nodiscard]] const std::string *get_string(const std::string &key) const noexcept;

        /// Returns a pointer to the name object for `key`,
        /// or `nullptr` if absent or not a name.
        [[nodiscard]] const name *get_name(const std::string &key) const noexcept;

        /// Returns a pointer to the array object for `key`,
        /// or `nullptr` if absent or not an array.
        [[nodiscard]] const array *get_array(const std::string &key) const noexcept;

        /// Returns a pointer to the nested dictionary object for `key`,
        /// or `nullptr` if absent or not a dictionary.
        [[nodiscard]] const dictionary *get_dictionary(const std::string &key) const noexcept;

        /// Returns a pointer to the indirect reference object for `key`,
        /// or `nullptr` if absent or not an indirect reference.
        [[nodiscard]] const indirect_reference *get_indirect_reference(const std::string &key) const noexcept;

        /// Returns the raw underlying map for full traversal or serialization.
        [[nodiscard]] const dictionary_map_type &entries() const noexcept;

    private:
        dictionary_map_type entries_;
    };
}
