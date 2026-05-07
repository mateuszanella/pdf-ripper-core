#pragma once

#include <expected>

#include "core/document/document_structure.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class document_structure_parser
    {
    public:
        virtual ~document_structure_parser() = default;
        virtual std::expected<document_structure, error> parse() = 0;
    };
}
