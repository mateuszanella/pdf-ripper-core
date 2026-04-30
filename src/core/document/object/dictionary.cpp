#include "core/document/object/dictionary.hpp"

namespace ripper::core
{
    dictionary::dictionary(map_type entries) noexcept
        : entries_(std::move(entries))
    {
    }

    void dictionary::set(std::string key, value value)
    {
        entries_.insert_or_assign(std::move(key), std::move(value));
    }

    bool dictionary::remove(const std::string &key) noexcept
    {
        return entries_.erase(key) > 0;
    }

    bool dictionary::contains(const std::string &key) const noexcept
    {
        return entries_.contains(key);
    }

    std::size_t dictionary::size() const noexcept
    {
        return entries_.size();
    }

    bool dictionary::empty() const noexcept
    {
        return entries_.empty();
    }

    const value *dictionary::get(const std::string &key) const noexcept
    {
        const auto it = entries_.find(key);
        return it != entries_.end() ? &it->second : nullptr;
    }

    const bool *dictionary::get_bool(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_bool() : nullptr;
    }

    const std::int64_t *dictionary::get_integer(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_integer() : nullptr;
    }

    const double *dictionary::get_real(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_real() : nullptr;
    }

    const std::string *dictionary::get_string(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_string() : nullptr;
    }

    const name *dictionary::get_name(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_name() : nullptr;
    }

    const array *dictionary::get_array(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_array() : nullptr;
    }

    const dictionary *dictionary::get_dictionary(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_dictionary() : nullptr;
    }

    const indirect_reference *dictionary::get_indirect_reference(const std::string &key) const noexcept
    {
        const auto *value = get(key);
        return value ? value->as_indirect_reference() : nullptr;
    }

    const dictionary::map_type &dictionary::entries() const noexcept
    {
        return entries_;
    }
}
