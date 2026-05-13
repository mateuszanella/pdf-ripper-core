#pragma once

#include <cstdint>
#include <optional>

#include "core/document/identifier.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/value.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    /// A PDF trailer dictionary.
    ///
    /// Thin wrapper around `dictionary`. All typed accessors (size, root,
    /// prev, id) are derived on-demand from the underlying dictionary; there
    /// are no separately cached fields.
    ///
    class trailer
    {
    public:
        /// Construct a trailer from a parsed PDF dictionary.
        explicit trailer(dictionary dict);

        /// /Size — total number of objects in the cross-reference table.
        [[nodiscard]] std::uint64_t size() const;

        /// /Root — indirect reference to the document catalog.
        [[nodiscard]] std::optional<indirect_reference> root() const;

        /// /Prev — byte offset of the previous cross-reference section.
        [[nodiscard]] std::optional<std::uint64_t> prev() const;

        /// /ID — document identifier pair.
        [[nodiscard]] std::optional<identifier> id() const;

        /// Access the const reference to the underlying dictionary directly.
        [[nodiscard]] const class dictionary &dictionary() const;

        /// Access the mutable reference to the underlying dictionary directly.
        [[nodiscard]] class dictionary &dictionary();

    private:
        class dictionary dict_;
    };
}
