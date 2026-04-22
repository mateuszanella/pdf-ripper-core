#include "core/serializer/serializer_manager.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/document/header.hpp"
#include "core/error.hpp"
#include "core/serializer/header/default_header_serializer.hpp"

namespace ripper::core
{
    serializer_manager::serializer_manager(const document &doc)
        : document_{doc}
    {
    }

    void serializer_manager::set_header_serializer(std::unique_ptr<class header_serializer> value) noexcept
    {
        header_serializer_ = std::move(value);
    }

    header_serializer &serializer_manager::header_serializer()
    {
        if (!header_serializer_)
            header_serializer_ = std::make_unique<class default_header_serializer>();

        return *header_serializer_;
    }
}
