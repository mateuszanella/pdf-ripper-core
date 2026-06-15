#include "core/serializer/trailer/default_trailer_serializer.hpp"

#include "core/document/object/object.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/util/byte.hpp"

#include <string>
#include <vector>

namespace ripper::pdf::core
{
default_trailer_serializer::default_trailer_serializer(class object_serializer& object_serializer)
    : object_serializer_{&object_serializer}
{
}

std::vector<std::byte> default_trailer_serializer::serialize(const trailer& t,
                                                             std::uint64_t xref_offset) const
{
    std::vector<std::byte> out;

    byte::append_bytes(out, "trailer");
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, object_serializer_->serialize(object{t.dictionary()}));
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, "startxref");
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, std::to_string(xref_offset));
    byte::append_bytes(out, line_break_character_);
    byte::append_bytes(out, "%%EOF");
    byte::append_bytes(out, line_break_character_);

    return out;
}

void default_trailer_serializer::set_object_serializer(class object_serializer& serializer)
{
    object_serializer_ = &serializer;
}
} // namespace ripper::pdf::core
