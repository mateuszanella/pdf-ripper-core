#pragma once

#include <expected>
#include <string_view>

#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/pages/pages_parser.hpp"

namespace ripper::core
{
    class default_pages_parser : public pages_parser
    {
    public:
        default_pages_parser() = default;

        [[nodiscard]] std::expected<parsed_pages, parser_error> parse(std::string_view content) const override;
    };
}
