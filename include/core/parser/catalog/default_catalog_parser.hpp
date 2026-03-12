#pragma once

#include <expected>
#include <string_view>

#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/catalog_parser.hpp"

namespace ripper::core
{
    class default_catalog_parser : public catalog_parser
    {
    public:
        default_catalog_parser() = default;

        [[nodiscard]] std::expected<catalog_parse_result, parser_error> parse(
            std::string_view content,
            indirect_reference catalog_ref) const override;

    private:
        [[nodiscard]] static std::expected<catalog_parse_result, parser_error> parse_dictionary(
            std::string_view content,
            indirect_reference catalog_ref);
    };
}
