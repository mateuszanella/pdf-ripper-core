#pragma once

#include <expected>
#include <string_view>

#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    /**
     * @brief Interface for parsing trailer dictionaries.
     */
    class trailer_parser
    {
    public:
        virtual ~trailer_parser() = default;

        /**
         * @brief Parses a trailer dictionary from raw content.
         * @param content The raw trailer content (starting with "trailer" keyword)
         */
        [[nodiscard]] virtual std::expected<trailer, error> parse(std::string_view content) = 0;
    };
}
