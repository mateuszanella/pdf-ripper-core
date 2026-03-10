#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "core/document/Header.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class header_parser
    {
    public:
        explicit header_parser(reader &reader);

        [[nodiscard]] std::expected<header, parser_error> parse();

    private:
        reader &_reader;
    };
}
