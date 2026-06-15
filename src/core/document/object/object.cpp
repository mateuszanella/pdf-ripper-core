#include "ripper/pdf/core/document/object/object.hpp"

#include "ripper/pdf/core/document/object/indirect_reference.hpp"

namespace ripper::pdf::core
{
/// Value implementation

object::object() noexcept : value_(null{}) {}

object::object(bool value) noexcept : value_(value) {}

object::object(std::int64_t value) noexcept : value_(value) {}

object::object(double value) noexcept : value_(value) {}

object::object(std::string value) noexcept : value_(std::move(value)) {}

object::object(name value) noexcept : value_(std::move(value)) {}

object::object(array value) noexcept : value_(std::move(value)) {}

object::object(dictionary value) noexcept : value_(std::make_unique<dictionary>(std::move(value)))
{
}

object::object(object_stream value) noexcept
    : value_(std::make_unique<object_stream>(std::move(value)))
{
}

object::object(indirect_reference value) noexcept : value_(std::move(value)) {}

object::object(const object& other)
    : value_(std::visit(
          [](const auto& v) -> variant_type
          {
              using T = std::decay_t<decltype(v)>;

              if constexpr (std::is_same_v<T, std::unique_ptr<dictionary>>)
                  return std::make_unique<dictionary>(*v);
              else if constexpr (std::is_same_v<T, std::unique_ptr<object_stream>>)
                  return std::make_unique<object_stream>(*v);
              else
                  return v;
          },
          other.value_))
{
}

object& object::operator=(const object& other)
{
    if (this != &other)
        value_ = object(other).value_;
    return *this;
}

bool object::is_null() const noexcept
{
    return std::holds_alternative<null>(value_);
}

bool object::is_bool() const noexcept
{
    return std::holds_alternative<bool>(value_);
}

bool object::is_integer() const noexcept
{
    return std::holds_alternative<std::int64_t>(value_);
}

bool object::is_real() const noexcept
{
    return std::holds_alternative<double>(value_);
}

bool object::is_string() const noexcept
{
    return std::holds_alternative<std::string>(value_);
}

bool object::is_name() const noexcept
{
    return std::holds_alternative<name>(value_);
}

bool object::is_array() const noexcept
{
    return std::holds_alternative<array>(value_);
}

bool object::is_dictionary() const noexcept
{
    return std::holds_alternative<std::unique_ptr<dictionary>>(value_);
}

bool object::is_stream() const noexcept
{
    return std::holds_alternative<std::unique_ptr<object_stream>>(value_);
}

bool object::is_indirect_reference() const noexcept
{
    return std::holds_alternative<indirect_reference>(value_);
}

const bool* object::as_bool() const noexcept
{
    return std::get_if<bool>(&value_);
}

bool* object::as_bool() noexcept
{
    return std::get_if<bool>(&value_);
}

const std::int64_t* object::as_integer() const noexcept
{
    return std::get_if<std::int64_t>(&value_);
}

std::int64_t* object::as_integer() noexcept
{
    return std::get_if<std::int64_t>(&value_);
}

const double* object::as_real() const noexcept
{
    return std::get_if<double>(&value_);
}

double* object::as_real() noexcept
{
    return std::get_if<double>(&value_);
}

const std::string* object::as_string() const noexcept
{
    return std::get_if<std::string>(&value_);
}

std::string* object::as_string() noexcept
{
    return std::get_if<std::string>(&value_);
}

const name* object::as_name() const noexcept
{
    return std::get_if<name>(&value_);
}

name* object::as_name() noexcept
{
    return std::get_if<name>(&value_);
}

const array* object::as_array() const noexcept
{
    return std::get_if<array>(&value_);
}

array* object::as_array() noexcept
{
    return std::get_if<array>(&value_);
}

const dictionary* object::as_dictionary() const noexcept
{
    const auto* ptr = std::get_if<std::unique_ptr<dictionary>>(&value_);

    if (ptr)
        return ptr->get();

    const auto* stream_ptr = std::get_if<std::unique_ptr<object_stream>>(&value_);
    if (!stream_ptr)
        return nullptr;

    return stream_ptr->get() ? &stream_ptr->get()->dictionary() : nullptr;
}

dictionary* object::as_dictionary() noexcept
{
    auto* ptr = std::get_if<std::unique_ptr<dictionary>>(&value_);

    if (ptr)
        return ptr->get();

    auto* stream_ptr = std::get_if<std::unique_ptr<object_stream>>(&value_);
    if (!stream_ptr)
        return nullptr;

    return stream_ptr->get() ? &stream_ptr->get()->dictionary() : nullptr;
}

const object_stream* object::as_stream() const noexcept
{
    const auto* ptr = std::get_if<std::unique_ptr<object_stream>>(&value_);
    return ptr ? ptr->get() : nullptr;
}

object_stream* object::as_stream() noexcept
{
    auto* ptr = std::get_if<std::unique_ptr<object_stream>>(&value_);
    return ptr ? ptr->get() : nullptr;
}

const indirect_reference* object::as_indirect_reference() const noexcept
{
    return std::get_if<indirect_reference>(&value_);
}

indirect_reference* object::as_indirect_reference() noexcept
{
    return std::get_if<indirect_reference>(&value_);
}

const object::variant_type& object::variant() const noexcept
{
    return value_;
}

object::variant_type& object::variant() noexcept
{
    return value_;
}

/// Object stream implementation

object_stream::object_stream(class dictionary dict, class stream stream) noexcept
    : dict_(std::move(dict)), stream_(std::move(stream))
{
}

const dictionary& object_stream::dictionary() const noexcept
{
    return dict_;
}

dictionary& object_stream::dictionary() noexcept
{
    return dict_;
}

const stream& object_stream::stream() const noexcept
{
    return stream_;
}

stream& object_stream::stream() noexcept
{
    return stream_;
}

/// Dictionary implementation

dictionary::dictionary(dictionary_map_type entries) noexcept : entries_(std::move(entries)) {}

void dictionary::set(std::string key, object value)
{
    entries_.insert_or_assign(std::move(key), std::move(value));
}

bool dictionary::remove(const std::string& key) noexcept
{
    return entries_.erase(key) > 0;
}

bool dictionary::contains(const std::string& key) const noexcept
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

const object* dictionary::get(const std::string& key) const noexcept
{
    const auto it = entries_.find(key);
    return it != entries_.end() ? &it->second : nullptr;
}

object* dictionary::get(const std::string& key) noexcept
{
    const auto it = entries_.find(key);
    return it != entries_.end() ? &it->second : nullptr;
}

const bool* dictionary::get_bool(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_bool() : nullptr;
}

bool* dictionary::get_bool(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_bool() : nullptr;
}

const std::int64_t* dictionary::get_integer(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_integer() : nullptr;
}

std::int64_t* dictionary::get_integer(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_integer() : nullptr;
}

const double* dictionary::get_real(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_real() : nullptr;
}

double* dictionary::get_real(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_real() : nullptr;
}

const std::string* dictionary::get_string(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_string() : nullptr;
}

std::string* dictionary::get_string(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_string() : nullptr;
}

const name* dictionary::get_name(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_name() : nullptr;
}

name* dictionary::get_name(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_name() : nullptr;
}

const array* dictionary::get_array(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_array() : nullptr;
}

array* dictionary::get_array(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_array() : nullptr;
}

const dictionary* dictionary::get_dictionary(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_dictionary() : nullptr;
}

dictionary* dictionary::get_dictionary(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_dictionary() : nullptr;
}

const indirect_reference* dictionary::get_indirect_reference(const std::string& key) const noexcept
{
    const auto* object = get(key);
    return object ? object->as_indirect_reference() : nullptr;
}

indirect_reference* dictionary::get_indirect_reference(const std::string& key) noexcept
{
    auto* object = get(key);
    return object ? object->as_indirect_reference() : nullptr;
}

const dictionary::dictionary_map_type& dictionary::entries() const noexcept
{
    return entries_;
}

dictionary::dictionary_map_type& dictionary::entries() noexcept
{
    return entries_;
}
} // namespace ripper::pdf::core
