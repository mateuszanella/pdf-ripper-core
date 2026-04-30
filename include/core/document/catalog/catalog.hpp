#pragma once

#include <optional>

#include "core/document/catalog/pages/pages.hpp"
#include "core/document/object/object.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /// Represents the PDF document catalog (the /Root object).
    ///
    /// The catalog is the root of the document's object hierarchy and serves as
    /// the entry point for accessing all document-level structures such as the
    /// page tree and other document-wide resources.
    ///
    /// Child objects are resolved lazily via the owning document and cached
    /// after first access, so subsequent accesses do not require additional parsing.
    class catalog : public object
    {
    public:
        /// Construct a catalog from an existing object.
        explicit catalog(object obj) noexcept;

        /// Return the pages object representing the document's page tree.
        ///
        /// If `reader` is set, the pages object is parsed from the input upon first access and
        /// cached for subsequent accesses. Otherwise, a new pages object with default values is
        /// created and cached.
        [[nodiscard]] std::expected<class pages, error> pages();

    private:
        std::optional<class pages> pages_;
    };
}
