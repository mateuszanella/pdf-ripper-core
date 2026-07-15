#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"

#include <algorithm>
#include <memory>
#include <utility>

namespace ripper::pdf::core
{
cross_reference_manager::cross_reference_manager(std::vector<revision>& revisions) noexcept
    : revisions_{&revisions}
{
}

cross_reference_entry* cross_reference_manager::find(std::uint32_t object_number) noexcept
{
    for (auto it = revisions_->rbegin(); it != revisions_->rend(); ++it)
    {
        if (auto* e = it->section().find(object_number))
            return e;
    }

    return nullptr;
}

const cross_reference_entry*
cross_reference_manager::find(std::uint32_t object_number) const noexcept
{
    for (auto it = revisions_->rbegin(); it != revisions_->rend(); ++it)
    {
        if (const auto* e = it->section().find(object_number))
            return e;
    }

    return nullptr;
}

cross_reference_entry* cross_reference_manager::find(const indirect_reference& ref) noexcept
{
    for (auto it = revisions_->rbegin(); it != revisions_->rend(); ++it)
    {
        if (auto* e = it->section().find(ref))
            return e;
    }

    return nullptr;
}

const cross_reference_entry*
cross_reference_manager::find(const indirect_reference& ref) const noexcept
{
    for (auto it = revisions_->rbegin(); it != revisions_->rend(); ++it)
    {
        if (const auto* e = it->section().find(ref))
            return e;
    }

    return nullptr;
}

indirect_reference cross_reference_manager::reserve()
{
    const std::uint32_t number = next_object_number();

    indirect_reference ref{number, 0};

    active_section().add_entry(cross_reference_entry{ref});

    return ref;
}

class indirect_object*
cross_reference_manager::commit(const indirect_reference& ref,
                                std::unique_ptr<class indirect_object> object) noexcept
{
    auto* entry = find(ref);
    if (entry == nullptr)
        return nullptr;

    if (!entry->is_new() || entry->is_resolved())
        return nullptr;

    return entry->resolve(std::move(object));
}

indirect_reference cross_reference_manager::allocate(std::unique_ptr<class indirect_object> object)
{
    const std::uint32_t number = next_object_number();

    indirect_reference ref{number, 0};

    active_section().add_entry(cross_reference_entry{ref, std::move(object)});

    return ref;
}

std::map<std::uint32_t, cross_reference_entry*> cross_reference_manager::entries()
{
    std::map<std::uint32_t, cross_reference_entry*> result;
    for (auto& rev : *revisions_)
    {
        for (auto& [num, ptr] : rev.section().entries())
            result.insert_or_assign(num, ptr);
    }

    return result;
}

std::map<std::uint32_t, cross_reference_entry*> cross_reference_manager::active_entries()
{
    auto all = entries();
    for (auto it = all.begin(); it != all.end();)
    {
        if (it->first != 0 && !it->second->in_use())
            it = all.erase(it);
        else
            ++it;
    }
    return all;
}

std::size_t cross_reference_manager::size() const noexcept
{
    std::size_t total = 0;
    for (const auto& rev : *revisions_)
        total += rev.section().size();

    return total;
}

std::uint32_t cross_reference_manager::next_object_number() const noexcept
{
    std::uint32_t result = 1;
    for (const auto& rev : *revisions_)
        result = std::max(result, rev.section().next_object_number());

    return result;
}

cross_reference_section& cross_reference_manager::active_section()
{
    return revisions_->back().section();
}
} // namespace ripper::pdf::core
