#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <expected>
#include <optional>

#include "core/error.hpp"
#include "core/error_builder.hpp"

namespace ripper::core
{
    std::expected<std::uint32_t, error> pages::count() const
    {
        if (!count_.has_value())
            return std::unexpected(error_builder::create()
                                       .with_code(error_code::internal_error)
                                       .with_component(error_component::pages)
                                       .with_field("Count")
                                       .with_expected("parsed page count")
                                       .with_actual("missing")
                                       .with_message("Page count not set in Pages object")
                                       .build());

        return *count_;
    }
}
