#include "core/document/catalog/entities/pages.hpp"

#include "core/document.hpp"

namespace ripper::core
{
    std::expected<std::uint32_t, parser_error> pages::count() const
    {
        return 0;
    }
}
