#pragma once

#include "ripper/pdf/core/document.hpp"

#include <optional>
#include <string>
#include <vector>

namespace ripper::pdf::core
{
class header_parser
{
public:
    explicit header_parser(const document& document);

    [[nodiscard]] header parse();

private:
    const document& _document;
};
} // namespace ripper::pdf::core
