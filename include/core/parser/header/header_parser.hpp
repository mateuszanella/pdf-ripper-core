#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "core/document.hpp"

namespace ripper::core
{
    class header_parser
    {
    public:
        explicit header_parser(const document &document);

        [[nodiscard]] std::expected<header, error> parse();

    private:
        const document &_document;
    };
}
