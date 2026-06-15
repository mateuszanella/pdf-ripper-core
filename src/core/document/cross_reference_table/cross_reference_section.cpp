#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"

#include <memory>

namespace ripper::pdf::core
{
cross_reference_section::cross_reference_section(
    std::vector<cross_reference_subsection> subsections,
    std::optional<std::uint64_t> startxref_offset) noexcept
    : subsections_{std::move(subsections)}, startxref_offset_{startxref_offset}
{
}

cross_reference_entry* cross_reference_section::find(std::uint32_t object_number) noexcept
{
    for (auto& sub : subsections_)
    {
        if (auto* e = sub.find(object_number))
            return e;
    }

    return nullptr;
}

const cross_reference_entry*
cross_reference_section::find(std::uint32_t object_number) const noexcept
{
    for (const auto& sub : subsections_)
    {
        if (const auto* e = sub.find(object_number))
            return e;
    }

    return nullptr;
}

cross_reference_entry* cross_reference_section::find(const indirect_reference& ref) noexcept
{
    return find(ref.object_number());
}

const cross_reference_entry*
cross_reference_section::find(const indirect_reference& ref) const noexcept
{
    return find(ref.object_number());
}

void cross_reference_section::add_entry(cross_reference_entry entry) noexcept
{
    const std::uint32_t num = entry.reference().object_number();

    if (!subsections_.empty())
    {
        auto& last = subsections_.back();
        const std::uint32_t expected_next = last.first_object_number() + last.count();
        if (num == expected_next)
        {
            last.entries().emplace(num, std::move(entry));
            return;
        }
    }

    cross_reference_subsection::entry_map new_entries;
    new_entries.emplace(num, std::move(entry));
    subsections_.emplace_back(num, std::move(new_entries));
}

indirect_reference cross_reference_section::reserve() noexcept
{
    const std::uint32_t number = next_object_number();

    indirect_reference ref{number, 0};

    add_entry(cross_reference_entry{ref});

    return ref;
}

class indirect_object*
cross_reference_section::commit(const indirect_reference& ref,
                                std::unique_ptr<class indirect_object> object) noexcept
{
    auto* entry = find(ref);
    if (!entry)
        return nullptr;

    if (!entry->is_new() || entry->is_resolved())
        return nullptr;

    return entry->resolve(std::move(object));
}

indirect_reference
cross_reference_section::allocate(std::unique_ptr<class indirect_object> object) noexcept
{
    const std::uint32_t number = next_object_number();

    indirect_reference ref{number, 0};

    add_entry(cross_reference_entry{ref, std::move(object)});

    return ref;
}

std::map<std::uint32_t, cross_reference_entry*> cross_reference_section::entries() noexcept
{
    std::map<std::uint32_t, cross_reference_entry*> result;
    for (auto& sub : subsections_)
    {
        for (auto& [num, entry] : sub.entries())
            result.emplace(num, &entry);
    }

    return result;
}

std::size_t cross_reference_section::size() const noexcept
{
    std::size_t total = 0;
    for (const auto& sub : subsections_)
        total += sub.count();

    return total;
}

std::uint32_t cross_reference_section::next_object_number() const noexcept
{
    std::uint32_t max_num = 0;
    for (const auto& sub : subsections_)
    {
        for (const auto& [num, entry] : sub.entries())
            max_num = std::max(max_num, num);
    }

    return max_num + 1;
}

const std::vector<cross_reference_subsection>& cross_reference_section::subsections() const noexcept
{
    return subsections_;
}

std::vector<cross_reference_subsection>& cross_reference_section::subsections() noexcept
{
    return subsections_;
}

std::optional<std::uint64_t> cross_reference_section::startxref_offset() const noexcept
{
    return startxref_offset_;
}

void cross_reference_section::set_startxref_offset(std::uint64_t offset) noexcept
{
    startxref_offset_ = offset;
}
} // namespace ripper::pdf::core
