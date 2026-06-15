#pragma once

#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/object/object_identity.hpp"

namespace ripper::pdf::core
{
/// A fully resolved PDF indirect object.
///
/// Composes identity and content to represent a PDF indirect object as defined in the spec:
///
///   - **Identity** (`object_identity`): the object number, generation number and owning document.
///   - **Content** (`object`): any PDF direct object — null, boolean, integer, real, string,
///     name, array, dictionary, or indirect reference.
///   - **Content stream**: represented as an `object` stream variant when applicable.
///
/// ## Relationship to `object_identity`
///
/// `object_identity` carries identity only — it knows *which* indirect_object this is, not *what*
/// it contains. `indirect_object` is the resolved form: it pairs that identity with a parsed
/// content object.
///
/// ## Derived types
///
/// Semantic PDF indirect object types (e.g. `catalog`, `pages`, `page`, `font`) extend this class
/// and provide domain-specific helpers on top of the raw content access provided here. They are
/// typed views over objects whose content is expected to be a dictionary; `dictionary()` returns
/// `nullptr` for objects whose content is a primitive.
///
/// ## Ownership
///
/// `indirect_object` owns its content object. The `object_identity` identity is held by value.
class indirect_object
{
public:
    /// Construct an indirect object with identity and content.
    indirect_object(object_identity identity, object content) noexcept;

    /// Returns the `object_identity` identity of this indirect_object, which includes the
    /// owning document and indirect reference.
    [[nodiscard]] const object_identity& identity() const noexcept;

    /// Returns a mutable reference to the `object_identity` identity of this indirect_object.
    [[nodiscard]] object_identity& identity() noexcept;

    /// Returns the raw content object of this indirect_object.
    ///
    /// May hold any PDF direct object type: null, boolean, integer, real, string,
    /// name, array, dictionary, or indirect reference.
    [[nodiscard]] const object& content() const noexcept;

    /// Returns a mutable reference to the raw content object.
    [[nodiscard]] object& content() noexcept;

    /// Returns a pointer to the content dictionary, or `nullptr` if the content is not a
    /// dictionary.
    ///
    /// Derived typed classes (catalog, pages, etc.) rely on this being non-null.
    [[nodiscard]] const class dictionary* dictionary() const noexcept;

    /// Returns a mutable pointer to the content dictionary, or `nullptr` if the content is not a
    /// dictionary.
    [[nodiscard]] class dictionary* dictionary() noexcept;

private:
    object_identity identity_;
    object content_;
};
} // namespace ripper::pdf::core
