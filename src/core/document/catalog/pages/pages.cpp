#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/object/object.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    pages::pages(object obj) noexcept
        : object{std::move(obj)}
    {
    }

    std::expected<std::uint64_t, error> pages::count() const
    {
        auto *d = dictionary();
        if (!d)
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_pages)
                                       .with_component(error_component::pages)
                                       .with_message("Pages content is not a dictionary")
                                       .build());

        auto count = d->get_integer("Count");
        if (!count)
        {
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::corrupted_pages)
                                       .with_component(error_component::pages)
                                       .with_field("Count")
                                       .with_expected("integer page count")
                                       .with_message("Pages object is missing required /Count entry")
                                       .build());
        }

        return static_cast<std::uint64_t>(*count);
    }
}
