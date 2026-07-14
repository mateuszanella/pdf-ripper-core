#include "ripper/pdf/core/serializer/cross_reference_table/compressed_cross_reference_table_serializer.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/object/stream.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/filter_manager.hpp"
#include "ripper/pdf/core/serializer/object/object_serializer.hpp"
#include "ripper/pdf/core/util/byte.hpp"

#include <string>

namespace ripper::pdf::core
{

void compressed_cross_reference_table_serializer::set_object_serializer(
    class object_serializer& serializer)
{
    object_serializer_ = &serializer;
}

std::vector<std::byte>
compressed_cross_reference_table_serializer::serialize(const cross_reference_section& section,
                                                       const trailer& trailer) const
{
    if (!section.is_compressed() || !section.xref_stream_object_number().has_value())
        throw logic_exception{"Compressed xref serializer requires a section with "
                              "xref_stream_object_number set"};

    const auto obj_num = *section.xref_stream_object_number();

    const auto widths = compute_widths(section);
    const auto entries_data = encode_entries(section, widths);

    std::uint32_t size = 0;
    for (const auto& sub : section.subsections())
    {
        const auto last = sub.first_object_number() + static_cast<std::uint32_t>(sub.count());
        if (last > size)
            size = last;
    }

    array w_arr;
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w0)});
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w1)});
    w_arr.push_back(object{static_cast<std::int64_t>(widths.w2)});

    // Start from the trailer dictionary so trailer-only entries (Root, Info, ID,
    // Encrypt, Prev) are merged into the xref stream dictionary per PDF spec
    // §7.5.8. The xref-specific keys (Type, Size, W, Index, Filter, Length)
    // are then set explicitly and take precedence over any trailer values.
    dictionary dict{trailer.dictionary().entries()};

    dict.set("Type", object{name{"XRef"}});
    dict.set("Size", object{static_cast<std::int64_t>(size)});
    dict.set("W", object{std::move(w_arr)});
    dict.set("Filter", object{name{"FlateDecode"}});

    const auto& subs = section.subsections();
    if (subs.size() != 1 || subs.front().first_object_number() != 0)
    {
        array idx;
        for (const auto& sub : subs)
        {
            idx.push_back(object{static_cast<std::int64_t>(sub.first_object_number())});
            idx.push_back(object{static_cast<std::int64_t>(sub.count())});
        }
        dict.set("Index", object{std::move(idx)});
    }
    else
    {
        // Remove any /Index the trailer might have carried; the section's
        // contiguous layout starting at object 0 does not need it.
        dict.remove("Index");
    }

    auto compressed = filter_manager::encode(dictionary{}, entries_data);
    dict.set("Length", object{static_cast<std::int64_t>(compressed.size())});

    if (object_serializer_ == nullptr)
        throw logic_exception{"Compressed xref serializer missing injected object_serializer"};

    auto body = object_serializer_->serialize(
        object{object_stream{std::move(dict), stream{std::move(compressed)}}});

    std::vector<std::byte> out;
    byte::append_bytes(out, std::to_string(obj_num) + " 0 obj\n");
    byte::append_bytes(out, body);
    byte::append_bytes(out, "\nendobj\n");

    return out;
}

compressed_cross_reference_table_serializer::column_widths
compressed_cross_reference_table_serializer::compute_widths(
    const cross_reference_section& section) const
{
    column_widths widths;

    for (const auto& sub : section.subsections())
    {
        for (const auto& [obj_num, entry] : sub.entries())
        {
            if (widths.w0 < 1)
                widths.w0 = 1;

            if (entry.type() == xref_entry_type::uncompressed)
            {
                auto val = entry.offset();

                while (val >= (1ULL << (widths.w1 * 8)))
                    ++widths.w1;

                auto gen = static_cast<std::uint64_t>(entry.reference().generation());

                while (gen >= (1ULL << (widths.w2 * 8)))
                    ++widths.w2;
            }
            if (entry.type() == xref_entry_type::compressed)
            {
                auto val = entry.objstm_number();
                while (val >= (1ULL << (widths.w1 * 8)))
                    ++widths.w1;
            }
            if (entry.type() == xref_entry_type::free)
            {
                auto val = entry.next_free_object();

                while (val >= (1ULL << (widths.w1 * 8)))
                    ++widths.w1;

                auto gen = static_cast<std::uint64_t>(entry.reuse_generation());

                while (gen >= (1ULL << (widths.w2 * 8)))
                    ++widths.w2;
            }

            if (entry.type() == xref_entry_type::compressed)
            {
                auto val = entry.objstm_index();
                while (val >= (1ULL << (widths.w2 * 8)))
                    ++widths.w2;
            }
        }
    }

    if (widths.w1 > 4)
        widths.w1 = 4;
    if (widths.w2 > 4)
        widths.w2 = 4;

    return widths;
}

std::vector<std::byte>
compressed_cross_reference_table_serializer::encode_entries(const cross_reference_section& section,
                                                            const column_widths& widths) const
{
    std::vector<std::byte> out;

    for (const auto& sub : section.subsections())
    {
        for (const auto& [obj_num, entry] : sub.entries())
        {
            write_field(out, static_cast<std::uint64_t>(entry.type()), widths.w0);

            switch (entry.type())
            {
                case xref_entry_type::free:
                    write_field(out, entry.next_free_object(), widths.w1);
                    break;
                case xref_entry_type::uncompressed:
                    write_field(out, entry.offset(), widths.w1);
                    break;
                case xref_entry_type::compressed:
                    write_field(out, entry.objstm_number(), widths.w1);
                    break;
            }

            switch (entry.type())
            {
                case xref_entry_type::free:
                    write_field(out, entry.reuse_generation(), widths.w2);
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
void compressed_cross_reference_table_serializer::write_field(std::vector<std::byte>& out,
                                                              std::uint64_t value,
                                                              std::uint32_t width)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
    for (std::int32_t i = static_cast<std::int32_t>(width) - 1; i >= 0; --i)
        out.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
}

} // namespace ripper::pdf::core
