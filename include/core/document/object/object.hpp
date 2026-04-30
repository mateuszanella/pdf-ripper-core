#pragma once

#include <optional>
#include <vector>
#include <cstddef>

#include "core/document/object/dictionary.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/object/stream.hpp"

namespace ripper::core
{
    /// A fully resolved PDF indirect object.
    ///
    /// Composes three components that together represent a PDF object as defined in the spec:
    ///
    ///   - **Identity** (`indirect_object`): the object number, generation number and owning document.
    ///   - **Dictionary** (`dictionary`): the metadata map describing the object (e.g. `/Type`, `/Pages`).
    ///   - **Content stream** (`stream`): optional raw byte payload, present only on stream objects.
    ///
    /// ## Relationship to `indirect_object`
    ///
    /// `indirect_object` carries identity only, it knows *which* object this is, not *what* it contains.
    /// `object` is the resolved form: it pairs that identity with a parsed dictionary and optional stream.
    ///
    /// ## Derived types
    ///
    /// Semantic PDF object types (e.g. `pages`, `page`, `font`) extend this class and provide
    /// domain-specific helpers on top of the raw dictionary and stream access provided here.
    ///
    /// ## Ownership
    ///
    /// `object` owns its dictionary and content stream. The `indirect_object` identity is held by value.
    class object
    {
    public:
        /// Construct an object with identity and dictionary, no content stream.
        object(indirect_object identity, dictionary dictionary) noexcept;

        /// Construct an object with identity, dictionary and a content stream.
        object(indirect_object identity, dictionary dictionary, stream stream) noexcept;

        virtual ~object() = default;

        object(const object &) = delete;
        object &operator=(const object &) = delete;

        object(object &&) = default;
        object &operator=(object &&) = default;

        /// Returns the `indirect_object` identity of this object, which includes the
        /// owning document and indirect reference.
        [[nodiscard]] const indirect_object &identity() const noexcept;

        /// Returns the dictionary holding this object's metadata.
        [[nodiscard]] const dictionary &dictionary() const noexcept;

        /// Returns a mutable reference to the dictionary.
        [[nodiscard]] class dictionary &dictionary() noexcept;

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
        class dictionary dictionary_;
        std::optional<class stream> stream_;
    };
}
