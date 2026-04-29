#include "core/document/object/value.hpp"

#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/dictionary.hpp"

namespace ripper::core
{
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
        : value_(std::move(value))
    {
    }

    value::value(indirect_reference value) noexcept
        : value_(std::move(value))
    {
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
        return std::holds_alternative<dictionary>(value_);
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
        return std::get_if<dictionary>(&value_);
    }

    const indirect_reference *value::as_indirect_reference() const noexcept
    {
        return std::get_if<indirect_reference>(&value_);
    }

    const value::variant_type &value::variant() const noexcept
    {
        return value_;
    }
}
