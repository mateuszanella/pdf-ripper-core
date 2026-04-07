#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string_view>

#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    struct parsed_pages
    {
        std::optional<std::uint32_t> count;
    };

    class pages_parser
    {
    public:
        virtual ~pages_parser() = default;

        [[nodiscard]] virtual std::expected<parsed_pages, parser_error> parse(std::string_view content) const = 0;
    };
}
