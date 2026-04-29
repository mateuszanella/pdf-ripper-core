#include "core/document/object/pdf_value.hpp"

namespace ripper::core
{
    pdf_value::pdf_value() noexcept
        : value_(pdf_null{})
    {}

    pdf_value::pdf_value(bool value) noexcept
        : value_(value)
    {}

    pdf_value::pdf_value(std::int64_t value) noexcept
        : value_(value)
    {}

    pdf_value::pdf_value(double value) noexcept
        : value_(value)
    {}

    pdf_value::pdf_value(std::string value) noexcept
        : value_(std::move(value))
    {}

    pdf_value::pdf_value(pdf_name value) noexcept
        : value_(std::move(value))
    {}

    pdf_value::pdf_value(pdf_array value) noexcept
        : value_(std::move(value))
    {}

    pdf_value::pdf_value(pdf_dictionary value) noexcept
        : value_(std::move(value))
    {}

    bool pdf_value::is_null() const noexcept
    {
        return std::holds_alternative<pdf_null>(value_);
    }

    bool pdf_value::is_bool() const noexcept
    {
        return std::holds_alternative<bool>(value_);
    }

    bool pdf_value::is_integer() const noexcept
    {
        return std::holds_alternative<std::int64_t>(value_);
    }

    bool pdf_value::is_real() const noexcept
    {
        return std::holds_alternative<double>(value_);
    }

    bool pdf_value::is_string() const noexcept
    {
        return std::holds_alternative<std::string>(value_);
    }

    bool pdf_value::is_name() const noexcept
    {
        return std::holds_alternative<pdf_name>(value_);
    }

    bool pdf_value::is_array() const noexcept
    {
        return std::holds_alternative<pdf_array>(value_);
    }

    bool pdf_value::is_dictionary() const noexcept
    {
        return std::holds_alternative<pdf_dictionary>(value_);
    }

    const bool *pdf_value::as_bool() const noexcept
    {
        return std::get_if<bool>(&value_);
    }

    const std::int64_t *pdf_value::as_integer() const noexcept
    {
        return std::get_if<std::int64_t>(&value_);
    }

    const double *pdf_value::as_real() const noexcept
    {
        return std::get_if<double>(&value_);
    }

    const std::string *pdf_value::as_string() const noexcept
    {
        return std::get_if<std::string>(&value_);
    }

    const pdf_name *pdf_value::as_name() const noexcept
    {
        return std::get_if<pdf_name>(&value_);
    }

    const pdf_array *pdf_value::as_array() const noexcept
    {
        return std::get_if<pdf_array>(&value_);
    }

    const pdf_dictionary *pdf_value::as_dictionary() const noexcept
    {
        return std::get_if<pdf_dictionary>(&value_);
    }

    const pdf_value::variant_type &pdf_value::variant() const noexcept
    {
        return value_;
    }
}
