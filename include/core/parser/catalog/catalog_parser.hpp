#pragma once

#include <expected>
#include <string_view>

#include "core/document/catalog/catalog.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class catalog_parser
    {
    public:
        virtual ~catalog_parser() = default;

        [[nodiscard]] virtual std::expected<catalog, error> parse(std::string_view content) const = 0;
    };
}
