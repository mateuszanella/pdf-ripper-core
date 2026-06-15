#pragma once

#include "core/document/document_structure.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
class document_structure_parser
{
public:
    virtual ~document_structure_parser() = default;
    virtual document_structure parse() = 0;
};
} // namespace ripper::pdf::core
