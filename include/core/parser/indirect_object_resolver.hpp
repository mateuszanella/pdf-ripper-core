#pragma once

#include <expected>
#include <string>

#include "core/document/object/indirect_reference.hpp"
#include "core/error.hpp"

namespace ripper::core
{
    class document;

    class indirect_object_resolver
    {
    public:
        explicit indirect_object_resolver(const document &document) noexcept;

        [[nodiscard]] std::expected<std::string, error> resolve(indirect_reference ref) const;

    private:
        const document &document_;
    };
}
