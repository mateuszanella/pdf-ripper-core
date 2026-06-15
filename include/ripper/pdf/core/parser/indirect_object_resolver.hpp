#pragma once

#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"

#include <string>

namespace ripper::pdf::core
{
class document;

class indirect_object_resolver
{
public:
    explicit indirect_object_resolver(document& document);

    [[nodiscard]] std::string resolve(indirect_reference ref) const;

private:
    document& document_;
};
} // namespace ripper::pdf::core
