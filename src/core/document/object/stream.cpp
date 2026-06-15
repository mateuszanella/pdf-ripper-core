#include "ripper/pdf/core/document/object/stream.hpp"

namespace ripper::pdf::core
{
stream::stream(std::vector<std::byte> data) noexcept
    : data_(std::move(data)), expected_size_{std::nullopt}, deferred_{false}
{
}

stream stream::deferred(std::size_t expected_size) noexcept
{
    stream result{std::vector<std::byte>{}};
    result.expected_size_ = expected_size;
    result.deferred_ = true;

    return result;
}

stream::stream(const stream& other)
    : data_{other.data_}, expected_size_{other.expected_size_}, deferred_{other.deferred_}
{
}

stream& stream::operator=(const stream& other)
{
    if (this == &other)
        return *this;

    data_ = other.data_;
    expected_size_ = other.expected_size_;
    deferred_ = other.deferred_;

    return *this;
}

stream::~stream() = default;

bool stream::is_deferred() const noexcept
{
    return deferred_;
}

bool stream::has_data() const noexcept
{
    return !data_.empty();
}

const std::vector<std::byte>& stream::data() const noexcept
{
    return data_;
}

std::vector<std::byte>& stream::data() noexcept
{
    return data_;
}

std::size_t stream::size() const noexcept
{
    return data_.size();
}

std::optional<std::size_t> stream::expected_size() const noexcept
{
    return expected_size_;
}

} // namespace ripper::pdf::core
