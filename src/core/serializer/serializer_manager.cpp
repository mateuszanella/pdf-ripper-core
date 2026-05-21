#include "core/serializer/serializer_manager.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/document/header.hpp"
#include "core/serializer/header/default_header_serializer.hpp"
#include "core/serializer/indirect_object/default_indirect_object_serializer.hpp"

namespace ripper::pdf::core
{
    serializer_manager::serializer_manager(const document &doc)
        : document_{doc}
    {
    }

    void serializer_manager::set_header_serializer(std::unique_ptr<class header_serializer> object)
    {
        header_serializer_ = std::move(object);
    }

    void serializer_manager::set_indirect_object_serializer(std::unique_ptr<class indirect_object_serializer> object)
    {
        indirect_object_serializer_ = std::move(object);
    }

    header_serializer &serializer_manager::header_serializer()
    {
        if (!header_serializer_)
            header_serializer_ = std::make_unique<class default_header_serializer>();

        return *header_serializer_;
    }

    indirect_object_serializer &serializer_manager::indirect_object_serializer()
    {
        if (!indirect_object_serializer_)
            indirect_object_serializer_ = std::make_unique<class default_indirect_object_serializer>();

        return *indirect_object_serializer_;
    }
}
