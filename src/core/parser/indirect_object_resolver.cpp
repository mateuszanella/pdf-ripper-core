#include "ripper/pdf/core/parser/indirect_object_resolver.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/util/text.hpp"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace ripper::pdf::core
{
indirect_object_resolver::indirect_object_resolver(document& document) : document_{document} {}

std::string indirect_object_resolver::resolve(indirect_reference ref) const
{
    auto* reader_ptr = document_.reader();
    if (reader_ptr == nullptr)
        throw io_exception{"No reader backend available"};

    auto& r = *reader_ptr;

    if (!r.is_open())
        throw io_exception{
            "The provided reader is not open while trying to resolve an indirect object"};

    auto& xref = document_.cross_reference_table();
    const auto entry = xref.find(ref);
    if (entry == nullptr)
        throw parse_exception{"XRef entry not found for indirect object"};

    if (!entry->in_use())
        throw parse_exception{"XRef entry is not in use"};

    const auto target_offset = entry->offset();
    if (!target_offset.has_value())
        throw parse_exception{"XRef entry is missing file offset"};

    const std::uint64_t file_size = r.size();
    if (file_size == 0)
        throw io_exception{"Read size zero while trying to resolve an indirect object"};

    if (*target_offset >= file_size)
        throw parse_exception{"Object offset is out of bounds"};

    // Determine the byte range occupied by this object on disk by examining
    // the xref: the object spans from its own file offset to the next larger
    // offset among all in-use entries and xref table start positions.
    std::optional<std::uint64_t> next_offset;

    for (auto& section : xref.sections())
    {
        for (const auto& entry_pair : section.entries())
        {
            const auto& off = entry_pair.second->offset();
            if (entry_pair.second->in_use() && off.has_value())
            {
                const auto off_val = *off;
                if (off_val > *target_offset)
                {
                    if (!next_offset.has_value() || off_val < *next_offset)
                        next_offset = off_val;
                }
            }
        }

        const auto sxref_off = section.startxref_offset();
        if (sxref_off.has_value())
        {
            const auto sx_val = *sxref_off;
            if (sx_val > *target_offset)
            {
                if (!next_offset.has_value() || sx_val < *next_offset)
                    next_offset = sx_val;
            }
        }
    }

    const std::uint64_t read_end = next_offset.value_or(file_size);
    const std::uint64_t read_size_u64 = read_end - *target_offset;

    if (read_size_u64 > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()))
        throw parse_exception{"Object is too large to handle"};

    const std::size_t read_size = static_cast<std::size_t>(read_size_u64);
    const std::size_t offset = static_cast<std::size_t>(*target_offset);

    std::vector<std::byte> bytes(read_size);
    const std::size_t bytes_read =
        r.read_at(std::span<std::byte>{bytes.data(), bytes.size()}, offset);
    if (bytes_read == 0)
        throw io_exception{
            "Received zero bytes from reader while attempting to read indirect object content"};

    std::string source(bytes_read, '\0');
    for (std::size_t i = 0; i < bytes_read; ++i)
        source[i] = static_cast<char>(bytes[i]);

    // Locate the object header. The xref offset points to the object number,
    // so " obj" should appear very early in the bounded source.
    const auto obj_kw = source.find(" obj");
    if (obj_kw == std::string::npos)
        throw parse_exception{"Missing obj marker for indirect object"};

    // Walk backwards from the " obj" marker to find the start of the object number.
    std::size_t header_start = obj_kw;
    while (header_start > 0 && source[header_start - 1] != '\n' && source[header_start - 1] != '\r')
        --header_start;

    std::size_t num_end = header_start;
    while (num_end < source.size() && std::isdigit(static_cast<unsigned char>(source[num_end])))
        ++num_end;

    const auto parsed_obj = text::parse_u32(source.substr(header_start, num_end - header_start));

    std::size_t gen_start = num_end;
    while (gen_start < source.size() && source[gen_start] == ' ')
        ++gen_start;

    std::size_t gen_end = gen_start;
    while (gen_end < source.size() && std::isdigit(static_cast<unsigned char>(source[gen_end])))
        ++gen_end;

    const auto parsed_gen = text::parse_u16(source.substr(gen_start, gen_end - gen_start));

    if (!parsed_obj || *parsed_obj != ref.object_number())
        throw parse_exception{"Object number mismatch in indirect object header"};

    if (!parsed_gen.has_value() && ref.generation() != 0)
        throw parse_exception{"Generation number mismatch in indirect object header"};

    if (parsed_gen.has_value() && *parsed_gen != ref.generation())
        throw parse_exception{"Generation number mismatch in indirect object header"};

    // Within the bounded range the object must end with "endobj".
    // rfind handles stream content that may contain arbitrary bytes.
    const auto endobj_pos = source.rfind("endobj");
    if (endobj_pos == std::string::npos)
        throw parse_exception{"Missing endobj marker for indirect object " +
                              std::to_string(ref.object_number()) + " (offset " +
                              std::to_string(*target_offset) + ", bound " +
                              std::to_string(read_end) + ")"};

    return source.substr(header_start, endobj_pos - header_start + 6);
}
} // namespace ripper::pdf::core
