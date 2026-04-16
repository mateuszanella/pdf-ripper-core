#pragma once

#include <expected>
#include <string_view>

#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/parser/trailer/trailer_parser.hpp"

namespace ripper::core
{
    /**
     * @brief Parses a single trailer dictionary.
     * Expects content starting with the "trailer" keyword.
     */
    class default_trailer_parser : public trailer_parser
    {
    public:
        default_trailer_parser() = default;

        [[nodiscard]] std::expected<trailer, error> parse(std::string_view content) override;

    private:
        [[nodiscard]] static std::expected<trailer, error> parse_dictionary(std::string_view content);

        [[nodiscard]] static std::expected<std::pair<std::uint32_t, std::uint16_t>, error>
            parse_indirect_reference(std::string_view line);
    };
}
