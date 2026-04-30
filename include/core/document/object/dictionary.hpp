#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "core/document/object/value.hpp"

namespace ripper::core
{
    /// A PDF dictionary object. An associative map of name keys to `value` objects.
    ///
    /// Wraps the raw `unordered_map<string, value>` representation with typed
    /// accessors and lookup helpers to reduce boilerplate in parsing and traversal code.
    ///
    /// ## Key convention
    ///
    /// Keys are stored without the leading `/` (e.g. `"Type"`, not `"/Type"`).
    /// This matches how parsed PDF names should be normalized before insertion.
    ///
    /// ## Access patterns
    ///
    /// Raw access via `get()` returns a pointer to the held `value`, or `nullptr`
    /// if the key is absent. Typed accessors (`get_name`, `get_integer`, etc.) combine
    /// the lookup and type-check into a single step, returning `nullptr` when either
    /// the key is missing or the value holds a different type.
    ///
    /// When full traversal is needed, `entries()` exposes the underlying map directly.
    class dictionary
    {
    public:
        /// Type alias for the underlying storage.
        using map_type = std::unordered_map<std::string, value>;

        /// Construct an empty dictionary.
        dictionary() noexcept = default;

        /// Construct a dictionary from an existing map.
        explicit dictionary(map_type entries) noexcept;

        /// Insert or overwrite a key-value pair.
        void set(std::string key, value value);

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

        /// Returns a pointer to the value for `key`, or `nullptr` if absent.
        [[nodiscard]] const value *get(const std::string &key) const noexcept;

        /// Returns a pointer to the boolean value for `key`,
        /// or `nullptr` if absent or not a boolean.
        [[nodiscard]] const bool *get_bool(const std::string &key) const noexcept;

        /// Returns a pointer to the integer value for `key`,
        /// or `nullptr` if absent or not an integer.
        [[nodiscard]] const std::int64_t *get_integer(const std::string &key) const noexcept;

        /// Returns a pointer to the real value for `key`,
        /// or `nullptr` if absent or not a real.
        [[nodiscard]] const double *get_real(const std::string &key) const noexcept;

        /// Returns a pointer to the string value for `key`,
        /// or `nullptr` if absent or not a string.
        [[nodiscard]] const std::string *get_string(const std::string &key) const noexcept;

        /// Returns a pointer to the name value for `key`,
        /// or `nullptr` if absent or not a name.
        [[nodiscard]] const name *get_name(const std::string &key) const noexcept;

        /// Returns a pointer to the array value for `key`,
        /// or `nullptr` if absent or not an array.
        [[nodiscard]] const array *get_array(const std::string &key) const noexcept;

        /// Returns a pointer to the nested dictionary value for `key`,
        /// or `nullptr` if absent or not a dictionary.
        [[nodiscard]] const dictionary *get_dictionary(const std::string &key) const noexcept;

        /// Returns a pointer to the indirect reference value for `key`,
        /// or `nullptr` if absent or not an indirect reference.
        [[nodiscard]] const indirect_reference *get_indirect_reference(const std::string &key) const noexcept;

        /// Returns the raw underlying map for full traversal or serialization.
        [[nodiscard]] const map_type &entries() const noexcept;

    private:
        map_type entries_;
    };
}
