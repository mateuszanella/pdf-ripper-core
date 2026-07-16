#pragma once

#include "ripper/pdf/core/document/revision_manager.hpp"

#include <memory>

namespace ripper::pdf::core
{
class revision_history_parser
{
public:
    virtual ~revision_history_parser() = default;

    virtual std::unique_ptr<revision_manager> parse() = 0;
};
} // namespace ripper::pdf::core
