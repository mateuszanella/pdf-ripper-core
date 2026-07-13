#pragma once

#include "ripper/pdf/core/document/document_revision.hpp"

namespace ripper::pdf::core
{
class document_revision_parser
{
public:
    virtual ~document_revision_parser() = default;

    virtual document_revision parse() = 0;
};
} // namespace ripper::pdf::core
