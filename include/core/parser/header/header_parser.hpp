#pragma once

#include <expected>
#include <string>
#include <optional>
#include <vector>

#include "core/document/header.hpp"
#include "core/error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class header_parser
    {
    public:
        explicit header_parser(reader &reader);

        [[nodiscard]] std::expected<header, error> parse();

    private:
        reader &_reader;
    };
}
