#include "core/serializer/trailer/default_trailer_serializer.hpp"

#include <string>
#include <vector>

#include "core/document/object/object.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/util/byte.hpp"

namespace ripper::pdf::core
{
    default_trailer_serializer::default_trailer_serializer(class object_serializer &object_serializer)
        : object_serializer_{&object_serializer}
    {
    }

    std::vector<std::byte> default_trailer_serializer::serialize(
        const trailer &t, std::uint64_t xref_offset) const
    {
        // Copy the dictionary and strip fields that must not appear in a full-save trailer.
        dictionary dict{t.dictionary().entries()};
        dict.remove("Prev");

        std::vector<std::byte> out;

        byte::append_bytes(out, "trailer\n");
        byte::append_bytes(out, object_serializer_->serialize(object{std::move(dict)}));
        byte::append_bytes(out, "startxref\n");
        byte::append_bytes(out, std::to_string(xref_offset));
        byte::append_bytes(out, "\n%%EOF\n");

        return out;
    }

    void default_trailer_serializer::set_object_serializer(class object_serializer &serializer)
    {
        object_serializer_ = &serializer;
    }
}
