#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"

#include "ripper/pdf/core/document/object/indirect_object.hpp"

namespace ripper::pdf::core
{

cross_reference_entry::cross_reference_entry(indirect_reference ref, std::uint64_t offset,
                                             bool in_use) noexcept
    : reference_{ref}, type_{in_use ? xref_entry_type::uncompressed : xref_entry_type::free},
      field1_{offset}, field2_{in_use ? 0U : static_cast<std::uint32_t>(ref.generation())},
      is_new_{false}, object_{nullptr}
{
}

cross_reference_entry::cross_reference_entry(indirect_reference ref) noexcept
    : reference_{ref}, type_{xref_entry_type::free}, field1_{0}, field2_{0}, is_new_{true},
      object_{nullptr}
{
}

cross_reference_entry::cross_reference_entry(indirect_reference ref,
                                             std::unique_ptr<class indirect_object> obj) noexcept
    : reference_{ref}, type_{xref_entry_type::uncompressed}, field1_{0}, field2_{0}, is_new_{true},
      object_{std::move(obj)}
{
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
cross_reference_entry::cross_reference_entry(indirect_reference ref, std::uint32_t objstm_number,
                                             std::uint32_t objstm_index) noexcept
    : reference_{ref}, type_{xref_entry_type::compressed},
      field1_{static_cast<std::uint64_t>(objstm_number)}, field2_{objstm_index}, is_new_{false},
      object_{nullptr}
{
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
cross_reference_entry cross_reference_entry::make_free(indirect_reference ref,
                                                       std::uint32_t next_free_obj,
                                                       std::uint16_t reuse_gen) noexcept
// NOLINTEND(bugprone-easily-swappable-parameters)
{
    cross_reference_entry entry{ref};
    entry.type_ = xref_entry_type::free;
    entry.field1_ = next_free_obj;
    entry.field2_ = reuse_gen;
    entry.is_new_ = false;
    return entry;
}

cross_reference_entry::cross_reference_entry(const cross_reference_entry& other)
    : reference_{other.reference_}, type_{other.type_}, field1_{other.field1_},
      field2_{other.field2_}, is_new_{other.is_new_},
      object_{other.object_ ? std::make_unique<class indirect_object>(other.object_->clone())
                            : nullptr}
{
}

cross_reference_entry& cross_reference_entry::operator=(const cross_reference_entry& other)
{
    if (this != &other)
    {
        reference_ = other.reference_;
        type_ = other.type_;
        field1_ = other.field1_;
        field2_ = other.field2_;
        is_new_ = other.is_new_;
        object_ = other.object_ ? std::make_unique<class indirect_object>(other.object_->clone())
                                : nullptr;
    }
    return *this;
}

const indirect_reference& cross_reference_entry::reference() const noexcept
{
    return reference_;
}

xref_entry_type cross_reference_entry::type() const noexcept
{
    return type_;
}

bool cross_reference_entry::in_use() const noexcept
{
    return type_ != xref_entry_type::free;
}

bool cross_reference_entry::is_compressed() const noexcept
{
    return type_ == xref_entry_type::compressed;
}

std::uint64_t cross_reference_entry::offset() const noexcept
{
    return field1_;
}

void cross_reference_entry::set_offset(std::uint64_t off) noexcept
{
    field1_ = off;
}

std::uint32_t cross_reference_entry::objstm_number() const noexcept
{
    return static_cast<std::uint32_t>(field1_);
}

std::uint32_t cross_reference_entry::objstm_index() const noexcept
{
    return field2_;
}

std::uint32_t cross_reference_entry::next_free_object() const noexcept
{
    return static_cast<std::uint32_t>(field1_);
}

std::uint16_t cross_reference_entry::reuse_generation() const noexcept
{
    return static_cast<std::uint16_t>(field2_);
}

bool cross_reference_entry::is_resolved() const noexcept
{
    return object_ != nullptr;
}

bool cross_reference_entry::is_new() const noexcept
{
    return is_new_;
}

indirect_object* cross_reference_entry::indirect_object() const noexcept
{
    return object_.get();
}

indirect_object* cross_reference_entry::resolve(std::unique_ptr<class indirect_object> obj) noexcept
{
    if (!obj)
        return nullptr;

    object_ = std::move(obj);
    if (type_ == xref_entry_type::free)
        type_ = xref_entry_type::uncompressed;

    return object_.get();
}

void cross_reference_entry::mark_deleted() noexcept
{
    type_ = xref_entry_type::free;
}

} // namespace ripper::pdf::core
