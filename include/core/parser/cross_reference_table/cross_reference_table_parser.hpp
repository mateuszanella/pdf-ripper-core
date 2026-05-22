#pragma once

#include <string_view>

#include "core/document/cross_reference_table/cross_reference_section.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
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
         * @brief Parses a cross-reference section from raw content.
         * @param content The raw xref content (starting with "xref" keyword)
         */
        [[nodiscard]] virtual cross_reference_section parse(std::string_view content) = 0;
    };
}
