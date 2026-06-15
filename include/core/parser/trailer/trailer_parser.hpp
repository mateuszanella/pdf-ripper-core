#pragma once

#include "core/document/trailer/trailer.hpp"
#include "core/exceptions/exception.hpp"

#include <string_view>

namespace ripper::pdf::core
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
    [[nodiscard]] virtual trailer parse(std::string_view content) = 0;
};
} // namespace ripper::pdf::core
