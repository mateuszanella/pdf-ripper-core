#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/object/object.hpp"
#include "core/error.hpp"
#include "core/errors/error_builder.hpp"

namespace ripper::core
{
    pages::pages(object object) noexcept
        : object{std::move(object)}
    {
    }

    std::expected<std::uint64_t, error> pages::count() const
    {
        auto count = dictionary().get_integer("Count");
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
