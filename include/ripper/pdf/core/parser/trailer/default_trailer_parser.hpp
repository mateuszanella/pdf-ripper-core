#pragma once

#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/parser/trailer/trailer_parser.hpp"

#include <string_view>

namespace ripper::pdf::core
{
/**
 * @brief Parses a single trailer dictionary.
 * Expects content starting with the "trailer" keyword.
 */
class default_trailer_parser : public trailer_parser
{
public:
    default_trailer_parser() = default;

    [[nodiscard]] trailer parse(std::string_view content) override;

private:
    [[nodiscard]] static trailer parse_dictionary(std::string_view content);
};
} // namespace ripper::pdf::core
