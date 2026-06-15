#include "core/document/cross_reference_table/cross_reference_subsection.hpp"

namespace ripper::pdf::core
{
cross_reference_subsection::cross_reference_subsection(std::uint32_t first_object_number,
                                                       entry_map entries) noexcept
    : first_object_number_{first_object_number}, entries_{std::move(entries)}
{
}

std::uint32_t cross_reference_subsection::first_object_number() const noexcept
{
    return first_object_number_;
}

std::uint32_t cross_reference_subsection::count() const noexcept
{
    return static_cast<std::uint32_t>(entries_.size());
}

const cross_reference_subsection::entry_map& cross_reference_subsection::entries() const noexcept
{
    return entries_;
}

cross_reference_subsection::entry_map& cross_reference_subsection::entries() noexcept
{
    return entries_;
}

cross_reference_entry* cross_reference_subsection::find(std::uint32_t object_number) noexcept
{
    auto it = entries_.find(object_number);
    if (it == entries_.end())
        return nullptr;

    return &it->second;
}

const cross_reference_entry*
cross_reference_subsection::find(std::uint32_t object_number) const noexcept
{
    auto it = entries_.find(object_number);
    if (it == entries_.end())
        return nullptr;

    return &it->second;
}
} // namespace ripper::pdf::core
