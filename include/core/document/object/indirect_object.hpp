#pragma once

#include <optional>
#include <vector>
#include <cstddef>

#include "core/document/object/stream.hpp"
#include "core/document/object/object_identity.hpp"
#include "core/document/object/object.hpp"

namespace ripper::pdf::core
{
    /// A fully resolved PDF indirect object.
    ///
    /// Composes three components that together represent a PDF indirect_object as defined in the spec:
    ///
    ///   - **Identity** (`object_identity`): the object number, generation number and owning document.
    ///   - **Content** (`object`): any PDF direct object — null, boolean, integer, real, string,
    ///     name, array, dictionary, or indirect reference.
    ///   - **Content stream** (`stream`): optional raw byte payload. Only meaningful when `content()`
    ///     holds a `dictionary`; a stream cannot be attached to a primitive object.
    ///
    /// ## Relationship to `object_identity`
    ///
    /// `object_identity` carries identity only — it knows *which* indirect_object this is, not *what* it contains.
    /// `indirect_object` is the resolved form: it pairs that identity with a parsed content object and optional stream.
    ///
    /// ## Derived types
    ///
    /// Semantic PDF indirect object types (e.g. `catalog`, `pages`, `page`, `font`) extend this class and
    /// provide domain-specific helpers on top of the raw content access provided here. They are
    /// typed views over objects whose content is expected to be a dictionary; `dictionary()` returns
    /// `nullptr` for objects whose content is a primitive.
    ///
    /// ## Ownership
    ///
    /// `indirect_object` owns its content object and content stream. The `object_identity` identity is held by value.
    class indirect_object
    {
    public:
        /// Construct an indirect_object with identity and content, no content stream.
        indirect_object(object_identity identity, object content) noexcept;

        /// Construct an indirect_object with identity, content and a content stream.
        ///
        /// A stream is only meaningful when `content` holds a `dictionary`.
        indirect_object(object_identity identity, object content, class stream stream) noexcept;

        /// Returns the `object_identity` identity of this indirect_object, which includes the
        /// owning document and indirect reference.
        [[nodiscard]] const object_identity &identity() const noexcept;

        /// Returns a mutable reference to the `object_identity` identity of this indirect_object.
        [[nodiscard]] object_identity &identity() noexcept;

        /// Returns the raw content object of this indirect_object.
        ///
        /// May hold any PDF direct object type: null, boolean, integer, real, string,
        /// name, array, dictionary, or indirect reference.
        [[nodiscard]] const object &content() const noexcept;

        /// Returns a mutable reference to the raw content object.
        [[nodiscard]] object &content() noexcept;

        /// Returns a pointer to the content dictionary, or `nullptr` if the content is not a dictionary.
        ///
        /// Derived typed classes (catalog, pages, etc.) rely on this being non-null.
        [[nodiscard]] const class dictionary *dictionary() const noexcept;

        /// Returns a mutable pointer to the content dictionary, or `nullptr` if the content is not a dictionary.
        [[nodiscard]] class dictionary *dictionary() noexcept;

        /// Returns `true` if this indirect_object carries a content stream.
        [[nodiscard]] bool has_stream() const noexcept;

        /// Returns a pointer to the content stream, or `nullptr` if this indirect_object has no stream.
        [[nodiscard]] const class stream *stream() const noexcept;

        /// Returns a mutable pointer to the content stream, or `nullptr` if this indirect_object has no stream.
        [[nodiscard]] class stream *stream() noexcept;

        /// Attach or replace the content stream on this indirect_object.
        void set_stream(class stream stream) noexcept;

    private:
        object_identity identity_;
        object content_;
        std::optional<class stream> stream_;
    };
}
