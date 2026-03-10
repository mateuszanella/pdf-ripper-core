#pragma once

#include <expected>
#include <string_view>

#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/catalog_parser.hpp"

namespace ripper::core
{
    /**
     * @brief Parses a catalog dictionary from an indirect object.
     */
    class default_catalog_parser : public catalog_parser
    {
    public:
        default_catalog_parser() = default;

        [[nodiscard]] std::expected<catalog, parser_error> parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<catalog, parser_error> parse_dictionary(std::string_view content);

        [[nodiscard]] static std::expected<std::pair<std::uint32_t, std::uint16_t>, parser_error>
            parse_indirect_reference(std::string_view line);
    };
}
