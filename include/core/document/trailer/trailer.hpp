#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/identifier.hpp"
#include "core/document/object/dictionary.hpp"
#include "core/document/object/indirect_reference.hpp"
#include "core/error.hpp"

namespace ripper::core
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
        explicit trailer(dictionary dict) noexcept;

        /// /Size — total number of objects in the cross-reference table.
        [[nodiscard]] std::expected<std::uint64_t, error> size() const noexcept;

        /// /Root — indirect reference to the document catalog.
        [[nodiscard]] std::expected<indirect_reference, error> root() const noexcept;

        /// /Prev — byte offset of the previous cross-reference section.
        [[nodiscard]] std::expected<std::uint64_t, error> prev() const noexcept;

        /// /ID — document identifier pair.
        [[nodiscard]] std::expected<identifier, error> id() const noexcept;

        /// Access the const reference to the underlying dictionary directly.
        [[nodiscard]] const class dictionary &dictionary() const noexcept;

        /// Access the mutable reference to the underlying dictionary directly.
        [[nodiscard]] class dictionary &dictionary() noexcept;

    private:
        class dictionary dict_;
    };
}
