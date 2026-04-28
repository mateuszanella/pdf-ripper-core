#pragma once

#include <optional>

#include "core/document/indirect_object.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /// Represents the PDF document catalog (the /Root object).
    ///
    /// The catalog is the root of the document's object hierarchy and serves as
    /// the entry point for accessing all document-level structures such as the
    /// page tree, metadata, and other document-wide resources.
    ///
    /// Child objects are resolved lazily via the owning document and cached
    /// after first access, so subsequent accesses do not require additional parsing.
    class catalog : public indirect_object
    {
    public:
        /// Construct a catalog from a document reference and indirect references.
        explicit catalog(const document &doc, indirect_reference ref, std::optional<indirect_reference> pages_ref) noexcept;

        /// Construct a catalog from an existing indirect object and an optional pages reference.
        explicit catalog(indirect_object obj, std::optional<indirect_reference> pages_ref) noexcept;

        /// Return the pages object representing the document's page tree.
        ///
        /// If `reader` is set, the pages object is parsed from the input upon first access and
        /// cached for subsequent accesses. Otherwise, a new pages object with default values is
        /// created and cached.
        [[nodiscard]] std::expected<class pages, error> pages();

    private:
        std::optional<indirect_reference> pages_ref_;
        std::optional<class pages> pages_;
    };
}
