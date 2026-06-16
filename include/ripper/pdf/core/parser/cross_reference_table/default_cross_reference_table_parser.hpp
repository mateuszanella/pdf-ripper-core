#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/cross_reference_table_parser.hpp"

#include <string_view>

namespace ripper::pdf::core
{
/**
 * @brief Parses a single traditional (non-compressed) cross-reference section.
 * Expects content starting with the "xref" keyword.
 */
class default_cross_reference_table_parser : public cross_reference_table_parser
{
public:
    default_cross_reference_table_parser() = default;

    [[nodiscard]] cross_reference_section parse(std::string_view content) override;

private:
    static void parse_subsection(cross_reference_section& section, std::string_view& content);
};
} // namespace ripper::pdf::core
