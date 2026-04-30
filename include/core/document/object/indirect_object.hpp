#pragma once

#include <cstdint>

#include "core/document/object/indirect_reference.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class document;

    /// Base class for any tangible PDF indirect object.
    ///
    /// An indirect object is a combination of an owned `indirect reference`
    /// (object number + generation number) and a non-owning `document` reference.
    ///
    /// All concrete PDF object types (pages, fonts, images, etc.) derive from
    /// this class and are associated with the owning `document`. The owner is
    /// accessible to derived classes through the protected `owner()` accessor.
    class indirect_object
    {
    public:
        /// Construct an indirect object bound to `doc` and identified by `ref`.
        ///
        /// The document is stored by reference and must outlive this object.
        indirect_object(const document &doc, indirect_reference ref) noexcept;

        /// Returns the indirect reference (object number + generation number) that
        /// uniquely identifies this object within its owning document.
        [[nodiscard]] const indirect_reference &reference() const noexcept;

        /// Returns the owning document.
        ///
        /// Derived classes may use this to navigate the document structure or
        /// resolve other indirect references.
        [[nodiscard]] const document &owner() const noexcept;

    private:
        const document &document_;
        indirect_reference reference_;
    };
}
