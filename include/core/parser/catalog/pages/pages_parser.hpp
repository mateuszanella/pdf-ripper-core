#pragma once

#include <expected>
#include <string_view>

#include "core/document/object/value.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class pages_parser
    {
    public:
        virtual ~pages_parser() = default;

        [[nodiscard]] virtual std::expected<dictionary, error> parse(std::string_view content) const = 0;
    };
}
