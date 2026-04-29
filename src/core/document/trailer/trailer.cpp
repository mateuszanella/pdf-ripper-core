#include "core/document/trailer/trailer.hpp"

#include "core/document/object/value.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    trailer::trailer(dictionary dict) noexcept
        : dict_{std::move(dict)}
    {
    }

    std::expected<std::uint64_t, error> trailer::size() const noexcept
    {
        const auto *v = dict_.get_integer("Size");
        if (!v)
            return std::unexpected(error_builder::create()
                .with_message("Trailer missing /Size entry")
                .with_code(error_code::not_found)
                .with_component(error_component::trailer)
                .build());

        return static_cast<std::uint64_t>(*v);
    }

    std::expected<indirect_reference, error> trailer::root() const noexcept
    {
        const auto *v = dict_.get("Root");
        if (!v)
            return std::unexpected(error_builder::create()
                .with_message("Trailer missing /Root entry")
                .with_code(error_code::not_found)
                .with_component(error_component::trailer)
                .build());

        const auto *ref = v->as_indirect_reference();
        if (!ref)
            return std::unexpected(error_builder::create()
                .with_message("Trailer /Root is not an indirect reference")
                .with_code(error_code::invalid_token)
                .with_component(error_component::trailer)
                .build());

        return *ref;
    }

    std::expected<std::uint64_t, error> trailer::prev() const noexcept
    {
        const auto *v = dict_.get_integer("Prev");
        if (!v)
            return std::unexpected(error_builder::create()
                .with_message("Trailer missing /Prev entry")
                .with_code(error_code::not_found)
                .with_component(error_component::trailer)
                .build());

        return static_cast<std::uint64_t>(*v);
    }

    std::expected<identifier, error> trailer::id() const noexcept
    {
        const auto *v = dict_.get_array("ID");
        if (!v || v->empty())
            return std::unexpected(error_builder::create()
                .with_message("Trailer missing or empty /ID entry")
                .with_code(error_code::not_found)
                .with_component(error_component::trailer)
                .build());

        const auto &arr = *v;
        const auto *orig = arr[0].as_string();
        if (!orig)
            return std::unexpected(error_builder::create()
                .with_message("Trailer /ID first element is not a string")
                .with_code(error_code::invalid_token)
                .with_component(error_component::trailer)
                .build());

        if (arr.size() >= 2)
        {
            const auto *curr = arr[1].as_string();
            if (curr)
                return identifier{*orig, *curr};
        }

        return identifier{*orig};
    }

    const dictionary &trailer::dictionary() const noexcept
    {
        return dict_;
    }

    dictionary &trailer::dictionary() noexcept
    {
        return dict_;
    }
}
