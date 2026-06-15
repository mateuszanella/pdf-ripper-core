#include "core/document/trailer/trailer.hpp"

#include "core/document/object/object.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
trailer::trailer(class dictionary dict) : dict_{std::move(dict)} {}

std::uint64_t trailer::size() const
{
    const auto* v = dict_.get_integer("Size");
    if (!v)
        throw parse_exception{"Trailer missing /Size entry"};

    return static_cast<std::uint64_t>(*v);
}

void trailer::set_size(std::uint64_t n)
{
    dict_.set("Size", object{static_cast<std::int64_t>(n)});
}

std::optional<indirect_reference> trailer::root() const
{
    const auto* v = dict_.get("Root");
    if (!v)
        return std::nullopt;

    const auto* ref = v->as_indirect_reference();
    if (!ref)
        throw parse_exception{"Trailer /Root is not an indirect reference"};

    return *ref;
}

std::optional<std::uint64_t> trailer::prev() const
{
    const auto* v = dict_.get_integer("Prev");
    if (!v)
        return std::nullopt;

    return static_cast<std::uint64_t>(*v);
}

std::optional<identifier> trailer::id() const
{
    const auto* v = dict_.get_array("ID");
    if (!v || v->empty())
        return std::nullopt;

    const auto& arr = *v;
    const auto* orig = arr[0].as_string();
    if (!orig)
        throw parse_exception{"Trailer /ID first element is not a string"};

    if (arr.size() >= 2)
    {
        const auto* curr = arr[1].as_string();
        if (curr)
            return identifier{*orig, *curr};
    }

    return identifier{*orig};
}

const dictionary& trailer::dictionary() const
{
    return dict_;
}

dictionary& trailer::dictionary()
{
    return dict_;
}
} // namespace ripper::pdf::core
