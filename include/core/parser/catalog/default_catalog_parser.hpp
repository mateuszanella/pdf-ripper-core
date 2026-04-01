#pragma once

#include <expected>
#include <string_view>

#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/catalog_parser.hpp"

namespace ripper::core
{
    class default_catalog_parser : public catalog_parser
    {
    public:
        default_catalog_parser() = default;

        [[nodiscard]] std::expected<catalog, parser_error> parse(std::string_view content) const override;
    };
}
