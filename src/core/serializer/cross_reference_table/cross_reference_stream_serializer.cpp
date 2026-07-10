#include "ripper/pdf/core/serializer/cross_reference_table/cross_reference_stream_serializer.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/filter/filter_manager.hpp"
#include "ripper/pdf/core/util/byte.hpp"

#include <string>

namespace ripper::pdf::core
{

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
std::vector<std::byte>
cross_reference_stream_serializer::serialize(const cross_reference_section& section,
                                             std::uint32_t obj_number, char line_break)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
    const auto widths = compute_widths(section);
    const auto entries_data = encode_entries(section, widths);

    // Build /W array
    array w_arr;
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w0)});
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w1)});
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w2)});

    // Build /Index array (omit if contiguous from 0)
    std::optional<array> index_arr;
    const auto& subsections = section.subsections();
    if (subsections.size() != 1 || subsections.front().first_object_number() != 0)
    {
        array idx;
        for (const auto& sub : subsections)
        {
            idx.push_back(object{static_cast<std::int64_t>(sub.first_object_number())});
            idx.push_back(object{static_cast<std::int64_t>(sub.count())});
        }
        index_arr = std::move(idx);
    }

    // Compute /Size (highest object number + 1)
    std::uint32_t size = 0;
    for (const auto& sub : subsections)
    {
        const auto last = sub.first_object_number() + static_cast<std::uint32_t>(sub.count());
        if (last > size)
            size = last;
    }

    // Compress with FlateDecode
    auto compressed = filter_manager::encode(dictionary{}, entries_data);

    // Build the stream dictionary
    dictionary dict;
    dict.set("Type", object{name{"XRef"}});
    dict.set("Size", object{static_cast<std::int64_t>(size)});
    dict.set("W", object{std::move(w_arr)});
    dict.set("Filter", object{name{"FlateDecode"}});
    dict.set("Length", object{static_cast<std::int64_t>(compressed.size())});

    // Serialize as indirect object
    std::vector<std::byte> out;

    // "N G obj"
    byte::append_bytes(out, std::to_string(obj_number));
    byte::append_bytes(out, " 0 obj");
    byte::append_bytes(out, line_break);

    // Dictionary
    byte::append_bytes(out, "<<");
    byte::append_bytes(out, line_break);

    byte::append_bytes(out, "/Type /XRef");
    byte::append_bytes(out, line_break);
    byte::append_bytes(out, "/Size ");
    byte::append_bytes(out, std::to_string(size));
    byte::append_bytes(out, line_break);

    // /W array
    byte::append_bytes(out, "/W [");
    byte::append_bytes(out, std::to_string(widths.w0));
    byte::append_bytes(out, " ");
    byte::append_bytes(out, std::to_string(widths.w1));
    byte::append_bytes(out, " ");
    byte::append_bytes(out, std::to_string(widths.w2));
    byte::append_bytes(out, "]");
    byte::append_bytes(out, line_break);

    // /Index array (if present)
    if (index_arr.has_value())
    {
        byte::append_bytes(out, "/Index [");
        for (std::size_t i = 0; i < index_arr->size(); ++i)
        {
            if (i > 0)
                byte::append_bytes(out, " ");
            if (const auto* elem = index_arr->at(i).as_integer())
                byte::append_bytes(out, std::to_string(*elem));
        }
        byte::append_bytes(out, "]");
        byte::append_bytes(out, line_break);
    }

    byte::append_bytes(out, "/Filter /FlateDecode");
    byte::append_bytes(out, line_break);
    byte::append_bytes(out, "/Length ");
    byte::append_bytes(out, std::to_string(compressed.size()));
    byte::append_bytes(out, line_break);

    byte::append_bytes(out, ">>");
    byte::append_bytes(out, line_break);

    // Stream
    byte::append_bytes(out, "stream");
    byte::append_bytes(out, line_break);
    byte::append_bytes(out, compressed);
    byte::append_bytes(out, line_break);
    byte::append_bytes(out, "endstream");
    byte::append_bytes(out, line_break);

    // "endobj"
    byte::append_bytes(out, "endobj");
    byte::append_bytes(out, line_break);

    return out;
}

cross_reference_stream_serializer::column_widths
cross_reference_stream_serializer::compute_widths(const cross_reference_section& section)
{
    column_widths widths;

    for (const auto& sub : section.subsections())
    {
        for (const auto& [obj_num, entry] : sub.entries())
        {
            // w0 is always 1 (type field)
            if (widths.w0 < 1)
                widths.w0 = 1;

            // w1: max of byte offset or objstm number
            if (entry.type() == xref_entry_type::uncompressed)
            {
                auto val = entry.offset();
                while (val >= (1ULL << (widths.w1 * 8)))
                    ++widths.w1;
            }
            if (entry.type() == xref_entry_type::compressed)
            {
                auto val = entry.objstm_number();
                while (val >= (1ULL << (widths.w1 * 8)))
                    ++widths.w1;
            }

            // w2: max of generation or objstm index
            if (entry.type() == xref_entry_type::compressed)
            {
                auto val = entry.objstm_index();
                while (val >= (1ULL << (widths.w2 * 8)))
                    ++widths.w2;
            }
        }
    }

    // Cap at reasonable limits
    if (widths.w1 > 4)
        widths.w1 = 4;
    if (widths.w2 > 4)
        widths.w2 = 4;

    return widths;
}

std::vector<std::byte>
cross_reference_stream_serializer::encode_entries(const cross_reference_section& section,
                                                  const column_widths& widths)
{
    std::vector<std::byte> out;

    for (const auto& sub : section.subsections())
    {
        for (const auto& [obj_num, entry] : sub.entries())
        {
            // Type field
            write_field(out, static_cast<std::uint64_t>(entry.type()), widths.w0);

            // Field 1
            switch (entry.type())
            {
                case xref_entry_type::free:
                    write_field(out, 0, widths.w1); // next free object (simplified)
                    break;
                case xref_entry_type::uncompressed:
                    write_field(out, entry.offset(), widths.w1);
                    break;
                case xref_entry_type::compressed:
                    write_field(out, entry.objstm_number(), widths.w1);
                    break;
            }

            // Field 2
            switch (entry.type())
            {
                case xref_entry_type::free:
                    write_field(out, 0, widths.w2); // next free generation (simplified)
                    break;
                case xref_entry_type::uncompressed:
                    write_field(out, entry.reference().generation(), widths.w2);
                    break;
                case xref_entry_type::compressed:
                    write_field(out, entry.objstm_index(), widths.w2);
                    break;
            }
        }
    }

    return out;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void cross_reference_stream_serializer::write_field(std::vector<std::byte>& out,
                                                    std::uint64_t value, std::uint32_t width)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
    for (std::int32_t i = static_cast<std::int32_t>(width) - 1; i >= 0; --i)
    {
        out.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
    }
}

} // namespace ripper::pdf::core
