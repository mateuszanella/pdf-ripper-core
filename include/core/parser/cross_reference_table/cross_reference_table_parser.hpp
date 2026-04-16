#pragma once

#include <expected>
#include <string_view>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /**
     * @brief Interface for parsing cross-reference tables.
     * Implementations handle different xref formats (traditional, compressed streams).
     */
    class cross_reference_table_parser
    {
    public:
        virtual ~cross_reference_table_parser() = default;

        /**
         * @brief Parses a cross-reference table from raw content.
         * @param content The raw xref content (starting with "xref" keyword)
         */
        [[nodiscard]] virtual std::expected<cross_reference_table, error> parse(std::string_view content) = 0;
    };
}
