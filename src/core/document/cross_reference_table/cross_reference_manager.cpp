#include "core/document/cross_reference_table/cross_reference_manager.hpp"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/document/cross_reference_table/cross_reference_entry.hpp"
#include "core/document/object/indirect_object.hpp"

namespace ripper::pdf::core
{
    cross_reference_manager::cross_reference_manager(
        std::vector<cross_reference_section> sections) noexcept
        : sections_{std::move(sections)}
    {
    }

    cross_reference_entry *cross_reference_manager::find(std::uint32_t object_number) noexcept
    {
        // Scan newest-to-oldest (back-to-front in chronological storage).
        for (auto it = sections_.rbegin(); it != sections_.rend(); ++it)
        {
            if (auto *e = it->find(object_number))
                return e;
        }

        return nullptr;
    }

    const cross_reference_entry *cross_reference_manager::find(std::uint32_t object_number) const noexcept
    {
        for (auto it = sections_.rbegin(); it != sections_.rend(); ++it)
        {
            if (const auto *e = it->find(object_number))
                return e;
        }

        return nullptr;
    }

    cross_reference_entry *cross_reference_manager::find(const indirect_reference &ref) noexcept
    {
        return find(ref.object_number());
    }

    const cross_reference_entry *cross_reference_manager::find(const indirect_reference &ref) const noexcept
    {
        return find(ref.object_number());
    }

    indirect_reference cross_reference_manager::reserve() noexcept
    {
        const std::uint32_t number = next_object_number();

        indirect_reference ref{number, 0};

        active_section().add_entry(cross_reference_entry{ref});

        return ref;
    }

    class indirect_object *cross_reference_manager::commit(
        const indirect_reference &ref,
        std::unique_ptr<class indirect_object> object) noexcept
    {
        auto *entry = find(ref);
        if (!entry)
            return nullptr;

        if (!entry->is_new() || entry->is_resolved())
            return nullptr;

        return entry->resolve(std::move(object));
    }

    indirect_reference cross_reference_manager::allocate(
        std::unique_ptr<class indirect_object> object) noexcept
    {
        const std::uint32_t number = next_object_number();

        indirect_reference ref{number, 0};

        active_section().add_entry(cross_reference_entry{ref, std::move(object)});

        return ref;
    }

    std::map<std::uint32_t, cross_reference_entry *> cross_reference_manager::entries() noexcept
    {
        std::map<std::uint32_t, cross_reference_entry *> result;
        // Iterate oldest-to-newest so that newer entries overwrite older ones.
        for (auto &section : sections_)
        {
            for (auto &[num, ptr] : section.entries())
                result.insert_or_assign(num, ptr);
        }

        return result;
    }

    std::map<std::uint32_t, cross_reference_entry *> cross_reference_manager::active_entries() noexcept
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
        for (const auto &section : sections_)
            total += section.size();

        return total;
    }

    std::uint32_t cross_reference_manager::next_object_number() const noexcept
    {
        std::uint32_t result = 1;
        for (const auto &section : sections_)
            result = std::max(result, section.next_object_number());

        return result;
    }

    const std::vector<cross_reference_section> &cross_reference_manager::sections() const noexcept
    {
        return sections_;
    }

    std::vector<cross_reference_section> &cross_reference_manager::sections() noexcept
    {
        return sections_;
    }

    cross_reference_section &cross_reference_manager::active_section() noexcept
    {
        if (sections_.empty())
            sections_.emplace_back(std::vector<cross_reference_subsection>{});

        return sections_.back();
    }

    void cross_reference_manager::squash() noexcept
    {
        auto active = active_entries();

        cross_reference_section consolidated{{}};

        // Move canonical entries in object-number order so add_entry's consecutive-grouping
        // logic produces a single compact subsection with no unnecessary gaps.
        for (auto &[num, ptr] : active)
            consolidated.add_entry(std::move(*ptr));

        // The old sections now hold only moved-from shells. Discard them.
        sections_.clear();
        sections_.push_back(std::move(consolidated));
    }
}
