#include "core/serializer/serializer.hpp"

#include "core/document.hpp"
#include "core/document/cross_reference_table/cross_reference_section.hpp"
#include "core/document/header.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/serializer/serializer_manager.hpp"

#include <cstdint>
#include <memory>
#include <utility>

namespace ripper::pdf::core
{
serializer::serializer(const document& doc)
    : document_{doc}, manager_{std::make_unique<class serializer_manager>(doc)}
{
}

serializer_manager& serializer::manager()
{
    if (!manager_)
        manager_ = std::make_unique<class serializer_manager>(document_);

    return *manager_;
}

void serializer::set_line_break_character(char c)
{
    manager().set_line_break_character(c);
}

void serializer::set_object_break_character(char c)
{
    manager().set_object_break_character(c);
}

std::vector<std::byte> serializer::serialize_header(const header& object)
{
    return manager().header_serializer().serialize(object);
}

std::vector<std::byte> serializer::serialize_indirect_object(const indirect_object& obj)
{
    return manager().indirect_object_serializer().serialize(obj);
}

std::vector<std::byte>
serializer::serialize_cross_reference_section(const cross_reference_section& section)
{
    return manager().cross_reference_table_serializer().serialize(section);
}

std::vector<std::byte> serializer::serialize_trailer(const trailer& t, std::uint64_t xref_offset)
{
    return manager().trailer_serializer().serialize(t, xref_offset);
}
} // namespace ripper::pdf::core
