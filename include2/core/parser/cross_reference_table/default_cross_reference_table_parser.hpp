#pragma once

#include <expected>
#include <string_view>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    /**
     * @brief Parses a single traditional (non-compressed) cross-reference table.
     * Expects content starting with the "xref" keyword.
     */
    class default_cross_reference_table_parser : public cross_reference_table_parser
    {
    public:
        default_cross_reference_table_parser() = default;

        [[nodiscard]] std::expected<cross_reference_table_parse_result, parser_error> parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<void, parser_error> parse_subsection(
            cross_reference_table &table,
            std::string_view &content);
    };
}
