#pragma once

#include <expected>
#include <string_view>

#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/object.hpp"
#include "core/error.hpp"

namespace ripper::pdf::core
{
    class document;

    /// Interface for parsing a single PDF indirect object from raw file bytes.
    ///
    /// The `content` passed to `parse()` is the raw byte slice starting at the
    /// xref-resolved byte offset for `ref`, i.e. it begins with `N G obj`.
    ///
    /// The returned `object` holds the fully parsed dictionary and, when present,
    /// the raw content stream bytes. The caller is responsible for casting the
    /// result to the appropriate typed subclass (catalog, pages, etc.).
    class object_parser
    {
    public:
        virtual ~object_parser() = default;

        [[nodiscard]] virtual std::expected<object, error> parse(
            document &doc,
            indirect_reference ref,
            std::string_view content) const = 0;
    };
}
