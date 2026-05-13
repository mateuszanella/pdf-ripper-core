#include "core/document/cross_reference_table/cross_reference_entry.hpp"

#include "core/document/object/indirect_object.hpp"
#include "core/document/object/object.hpp"

namespace ripper::pdf::core
{
    cross_reference_entry::cross_reference_entry(indirect_reference ref, std::uint64_t offset, bool in_use) noexcept
        : reference_{ref}, offset_{offset}, in_use_{in_use}, object_{nullptr}
    {
    }

    cross_reference_entry::cross_reference_entry(indirect_reference ref) noexcept
        : reference_{ref}, offset_{std::nullopt}, in_use_{false}, object_{nullptr}
    {
    }

    cross_reference_entry::cross_reference_entry(indirect_reference ref, std::unique_ptr<class object> obj) noexcept
        : reference_{ref}, offset_{std::nullopt}, in_use_{true}, object_{std::move(obj)}
    {
    }

    const indirect_reference &cross_reference_entry::reference() const noexcept
    {
        return reference_;
    }

    const std::optional<std::uint64_t> &cross_reference_entry::offset() const noexcept
    {
        return offset_;
    }

    bool cross_reference_entry::in_use() const noexcept
    {
        return in_use_;
    }

    bool cross_reference_entry::is_resolved() const noexcept
    {
        return object_ != nullptr;
    }

    bool cross_reference_entry::is_new() const noexcept
    {
        return !offset_.has_value();
    }

    object *cross_reference_entry::object() const noexcept
    {
        return object_.get();
    }

    object *cross_reference_entry::resolve(std::unique_ptr<class object> obj) noexcept
    {
        if (!obj)
            return nullptr;

        if (object_)
            return object_.get();

        object_ = std::move(obj);
        in_use_ = true;

        return object_.get();
    }
}
