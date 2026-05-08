#pragma once

#include <optional>
#include <vector>
#include <cstddef>

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/stream.hpp"
#include "core/document/object/value.hpp"

namespace ripper::core
{
    /// A fully resolved PDF indirect object.
    ///
    /// Composes three components that together represent a PDF object as defined in the spec:
    ///
    ///   - **Identity** (`indirect_object`): the object number, generation number and owning document.
    ///   - **Content** (`value`): any PDF direct object — null, boolean, integer, real, string,
    ///     name, array, dictionary, or indirect reference.
    ///   - **Content stream** (`stream`): optional raw byte payload. Only meaningful when `content()`
    ///     holds a `dictionary`; a stream cannot be attached to a primitive value.
    ///
    /// ## Relationship to `indirect_object`
    ///
    /// `indirect_object` carries identity only — it knows *which* object this is, not *what* it contains.
    /// `object` is the resolved form: it pairs that identity with a parsed content value and optional stream.
    ///
    /// ## Derived types
    ///
    /// Semantic PDF object types (e.g. `catalog`, `pages`, `page`, `font`) extend this class and
    /// provide domain-specific helpers on top of the raw content access provided here. They are
    /// typed views over objects whose content is expected to be a dictionary; `dictionary()` returns
    /// `nullptr` for objects whose content is a primitive.
    ///
    /// ## Ownership
    ///
    /// `object` owns its content value and content stream. The `indirect_object` identity is held by value.
    class object
    {
    public:
        /// Construct an object with identity and content, no content stream.
        object(indirect_object identity, value content) noexcept;

        /// Construct an object with identity, content and a content stream.
        ///
        /// A stream is only meaningful when `content` holds a `dictionary`.
        object(indirect_object identity, value content, class stream stream) noexcept;

        virtual ~object() = default;

        object(const object &) = default;
        object &operator=(const object &) = delete;

        object(object &&) = default;
        object &operator=(object &&) = delete;

        /// Returns the `indirect_object` identity of this object, which includes the
        /// owning document and indirect reference.
        [[nodiscard]] const indirect_object &identity() const noexcept;

        /// Returns the raw content value of this object.
        ///
        /// May hold any PDF direct object type: null, boolean, integer, real, string,
        /// name, array, dictionary, or indirect reference.
        [[nodiscard]] const value &content() const noexcept;

        /// Returns a mutable reference to the raw content value.
        [[nodiscard]] value &content() noexcept;

        /// Returns a pointer to the content dictionary, or `nullptr` if the content is not a dictionary.
        ///
        /// Derived typed classes (catalog, pages, etc.) rely on this being non-null.
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

        /// Returns a mutable pointer to the content dictionary, or `nullptr` if the content is not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;

        /// Returns `true` if this object carries a content stream.
        [[nodiscard]] bool has_stream() const noexcept;

        /// Returns a pointer to the content stream, or `nullptr` if this object has no stream.
        [[nodiscard]] const class stream *stream() const noexcept;

        /// Returns a mutable pointer to the content stream, or `nullptr` if this object has no stream.
        [[nodiscard]] class stream *stream() noexcept;

        /// Attach or replace the content stream on this object.
        void set_stream(class stream stream) noexcept;

    private:
        indirect_object identity_;
        value content_;
        std::optional<class stream> stream_;
    };
}
