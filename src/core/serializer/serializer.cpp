#include "core/serializer/serializer.hpp"

#include <memory>
#include <utility>

#include "core/document.hpp"
#include "core/document/header.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/serializer/serializer_manager.hpp"

namespace ripper::pdf::core
{
    serializer::serializer(const document &doc)
        : document_{doc},
          manager_{std::make_unique<class serializer_manager>(doc)}
    {
    }

    serializer_manager &serializer::manager()
    {
        if (!manager_)
            manager_ = std::make_unique<class serializer_manager>(document_);

        return *manager_;
    }

    std::vector<std::byte> serializer::serialize_header(const header &object)
    {
        return manager().header_serializer().serialize(object);
    }

    std::vector<std::byte> serializer::serialize_indirect_object(const indirect_object &obj)
    {
        return manager().indirect_object_serializer().serialize(obj);
    }
}
