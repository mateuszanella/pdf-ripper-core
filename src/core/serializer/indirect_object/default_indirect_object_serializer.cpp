#include "ripper/pdf/core/serializer/indirect_object/default_indirect_object_serializer.hpp"

#include "ripper/pdf/core/document/object/helpers/indirect_object.hpp"
#include "ripper/pdf/core/document/object/helpers/object_identity.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/util/byte.hpp"

#include <string>
#include <vector>

namespace ripper::pdf::core
{
default_indirect_object_serializer::default_indirect_object_serializer(
    class object_serializer& object_serializer)
    : object_serializer_{&object_serializer}
{
}

std::vector<std::byte>
default_indirect_object_serializer::serialize(const indirect_object& obj) const
{
    std::vector<std::byte> out;

    byte::append_bytes(out, serialize_direct_reference(obj.identity().reference()));
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, object_serializer_->serialize(obj.content()));
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, "endobj");
    byte::append_bytes(out, line_break_character_);

    return out;
}

void default_indirect_object_serializer::set_object_serializer(class object_serializer& serializer)
{
    object_serializer_ = &serializer;
}

void default_indirect_object_serializer::set_line_break_character(char c)
{
    indirect_object_serializer::set_line_break_character(c);
    object_serializer_->set_line_break_character(c);
}

void default_indirect_object_serializer::set_object_break_character(char c)
{
    indirect_object_serializer::set_object_break_character(c);
    object_serializer_->set_object_break_character(c);
}

std::vector<std::byte> default_indirect_object_serializer::serialize_direct_reference(
    const indirect_reference& value) const
{
    std::vector<std::byte> out;
    byte::append_bytes(out, std::to_string(value.object_number()));
    byte::append_bytes(out, ' ');
    byte::append_bytes(out, std::to_string(value.generation()));
    byte::append_bytes(out, ' ');
    byte::append_bytes(out, "obj");

    return out;
}
} // namespace ripper::pdf::core
