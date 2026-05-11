#include "core/document/object/value.hpp"

#include "core/document/object/indirect_reference.hpp"

namespace ripper::pdf::core
{
    /// Value implementation

    value::value() noexcept
        : value_(null{})
    {
    }

    value::value(bool value) noexcept
        : value_(value)
    {
    }

    value::value(std::int64_t value) noexcept
        : value_(value)
    {
    }

    value::value(double value) noexcept
        : value_(value)
    {
    }

    value::value(std::string value) noexcept
        : value_(std::move(value))
    {
    }

    value::value(name value) noexcept
        : value_(std::move(value))
    {
    }

    value::value(array value) noexcept
        : value_(std::move(value))
    {
    }

    value::value(dictionary value) noexcept
        : value_(std::make_unique<dictionary>(std::move(value)))
    {
    }

    value::value(indirect_reference value) noexcept
        : value_(std::move(value))
    {
    }

    value::value(const value &other)
        : value_(std::visit([](const auto &v) -> variant_type
                            {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<dictionary>>)
            return std::make_unique<dictionary>(*v);
        else
            return v; }, other.value_))
    {
    }

    value &value::operator=(const value &other)
    {
        if (this != &other)
            value_ = value(other).value_;
        return *this;
    }

    bool value::is_null() const noexcept
    {
        return std::holds_alternative<null>(value_);
    }

    bool value::is_bool() const noexcept
    {
        return std::holds_alternative<bool>(value_);
    }

    bool value::is_integer() const noexcept
    {
        return std::holds_alternative<std::int64_t>(value_);
    }

    bool value::is_real() const noexcept
    {
        return std::holds_alternative<double>(value_);
    }

    bool value::is_string() const noexcept
    {
        return std::holds_alternative<std::string>(value_);
    }

    bool value::is_name() const noexcept
    {
        return std::holds_alternative<name>(value_);
    }

    bool value::is_array() const noexcept
    {
        return std::holds_alternative<array>(value_);
    }

    bool value::is_dictionary() const noexcept
    {
        return std::holds_alternative<std::unique_ptr<dictionary>>(value_);
    }

    bool value::is_indirect_reference() const noexcept
    {
        return std::holds_alternative<indirect_reference>(value_);
    }

    const bool *value::as_bool() const noexcept
    {
        return std::get_if<bool>(&value_);
    }

    const std::int64_t *value::as_integer() const noexcept
    {
        return std::get_if<std::int64_t>(&value_);
    }

    const double *value::as_real() const noexcept
    {
        return std::get_if<double>(&value_);
    }

    const std::string *value::as_string() const noexcept
    {
        return std::get_if<std::string>(&value_);
    }

    const name *value::as_name() const noexcept
    {
        return std::get_if<name>(&value_);
    }

    const array *value::as_array() const noexcept
    {
        return std::get_if<array>(&value_);
    }

    const dictionary *value::as_dictionary() const noexcept
    {
        const auto *ptr = std::get_if<std::unique_ptr<dictionary>>(&value_);

        return ptr ? ptr->get() : nullptr;
    }

    dictionary *value::as_dictionary() noexcept
    {
        auto *ptr = std::get_if<std::unique_ptr<dictionary>>(&value_);

        return ptr ? ptr->get() : nullptr;
    }

    const indirect_reference *value::as_indirect_reference() const noexcept
    {
        return std::get_if<indirect_reference>(&value_);
    }

    const value::variant_type &value::variant() const noexcept
    {
        return value_;
    }

    /// Dictionary implementation

    dictionary::dictionary(dictionary_map_type entries) noexcept
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

    const dictionary::dictionary_map_type &dictionary::entries() const noexcept
    {
        return entries_;
    }
}
