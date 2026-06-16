#pragma once

#include "ripper/pdf/core/document/identifier.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/document/object/object.hpp"

#include <cstdint>
#include <optional>

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

    /// Sets /Size. Should equal highest object number + 1.
    void set_size(std::uint64_t n);

    /// /Root — indirect reference to the document catalog.
    [[nodiscard]] std::optional<indirect_reference> root() const;

    /// /Prev — byte offset of the previous cross-reference section.
    [[nodiscard]] std::optional<std::uint64_t> prev() const;

    /// /ID — document identifier pair.
    [[nodiscard]] std::optional<identifier> id() const;

    /// Access the const reference to the underlying dictionary directly.
    [[nodiscard]] const class dictionary& dictionary() const;

    /// Access the mutable reference to the underlying dictionary directly.
    [[nodiscard]] class dictionary& dictionary();

private:
    class dictionary dict_;
};
} // namespace ripper::pdf::core
