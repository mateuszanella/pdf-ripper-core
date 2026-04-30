#include "core/document/cross_reference_table/cross_reference_table.hpp"

#include "core/document/object/indirect_object.hpp"

namespace ripper::core
{
    cross_reference_table::cross_reference_table(entry_map entries) noexcept
        : entries_{std::move(entries)}
    {
    }

    const cross_reference_table::entry_map &cross_reference_table::entries() const noexcept
    {
        return entries_;
    }

    cross_reference_table::entry_map &cross_reference_table::entries() noexcept
    {
        return entries_;
    }

    cross_reference_entry *cross_reference_table::find(std::uint32_t object_number) const noexcept
    {
        auto it = entries_.find(object_number);
        if (it == entries_.end())
            return nullptr;

        return const_cast<cross_reference_entry *>(&it->second);
    }

    cross_reference_entry *cross_reference_table::find(const indirect_reference &ref) const noexcept
    {
        return find(ref.object_number());
    }

    class object *cross_reference_table::resolve(const indirect_reference &ref, const loader_fn &loader) const
    {
        auto it = entries_.find(ref.object_number());
        if (it == entries_.end())
            return nullptr;

        auto &entry = it->second;

        if (!entry.is_resolved())
        {
            auto obj = loader(entry);
            if (obj)
                return entry.resolve(std::move(obj));
        }

        return entry.object();
    }

    indirect_reference cross_reference_table::reserve() noexcept
    {
        std::uint32_t number = next_object_number();
        indirect_reference ref{number, 0};
        entries_.emplace(number, cross_reference_entry{ref});
        return ref;
    }

    class object *cross_reference_table::commit(const indirect_reference &ref, std::unique_ptr<class object> object) noexcept
    {
        auto it = entries_.find(ref.object_number());
        if (it == entries_.end())
            return nullptr;

        return it->second.resolve(std::move(object));
    }

    indirect_reference cross_reference_table::allocate(std::unique_ptr<class object> object) noexcept
    {
        std::uint32_t number = next_object_number();
        indirect_reference ref{number, 0};

        entries_.emplace(number, cross_reference_entry{ref, std::move(object)});

        return ref;
    }

    std::size_t cross_reference_table::size() const noexcept
    {
        return entries_.size();
    }

    std::uint32_t cross_reference_table::next_object_number() const noexcept
    {
        if (entries_.empty())
            return 1;

        return static_cast<std::uint32_t>(entries_.size()) + 1;
    }
}
