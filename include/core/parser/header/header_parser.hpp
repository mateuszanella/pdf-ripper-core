#pragma once

#include <string>
#include <optional>
#include <vector>

#include "core/document.hpp"

namespace ripper::pdf::core
{
    class header_parser
    {
    public:
        explicit header_parser(const document &document);

        [[nodiscard]] header parse();

    private:
        const document &_document;
    };
}
