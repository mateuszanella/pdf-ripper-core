#pragma once

#include "ripper/pdf/core/document/document_structure.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
class document_structure_parser
{
public:
    virtual ~document_structure_parser() = default;
    virtual document_structure parse() = 0;
};
} // namespace ripper::pdf::core
