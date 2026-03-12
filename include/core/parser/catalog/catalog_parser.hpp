#pragma once

#include <expected>
#include <optional>
#include <string_view>

#include "core/document/indirect_reference.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    struct catalog_parse_result
    {
        indirect_reference catalog_ref;
        std::optional<indirect_reference> pages_ref;
    };

    class catalog_parser
    {
    public:
        virtual ~catalog_parser() = default;

        [[nodiscard]] virtual std::expected<catalog_parse_result, parser_error> parse(
            std::string_view content,
            indirect_reference catalog_ref) const = 0;
    };
}
