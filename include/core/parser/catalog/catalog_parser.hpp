#pragma once

#include <expected>
#include <string_view>

#include "core/document/indirect_reference.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    class catalog_parser
    {
    public:
        virtual ~catalog_parser() = default;

        [[nodiscard]] virtual std::expected<catalog, parser_error> parse(std::string_view content) const = 0;
    };
}
