#pragma once

#include <expected>
#include <optional>
#include <string>
#include <string_view>

#include "core/document/indirect_reference.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    struct parsed_catalog
    {
        std::optional<indirect_reference> pages_ref;
        std::optional<std::string> version;
    };

    class catalog_parser
    {
    public:
        virtual ~catalog_parser() = default;

        [[nodiscard]] virtual std::expected<parsed_catalog, parser_error> parse(std::string_view content) const = 0;
    };
}
