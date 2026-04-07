#include "core/document/catalog/pages/pages.hpp"

#include <cstdint>
#include <expected>
#include <optional>

#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    std::expected<std::uint32_t, parser_error> pages::count() const
    {
        if (!count_.has_value())
            return std::unexpected(parser_error::corrupted_object);

        return *count_;
    }
}
