#pragma once

#include <string>

#include "core/document/object/indirect_reference.hpp"
#include "core/exceptions/exception.hpp"

namespace ripper::pdf::core
{
    class document;

    class indirect_object_resolver
    {
    public:
        explicit indirect_object_resolver(document &document);

        [[nodiscard]] std::string resolve(indirect_reference ref) const;

    private:
        document &document_;
    };
}
