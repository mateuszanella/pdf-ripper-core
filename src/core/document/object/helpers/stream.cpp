#include "ripper/pdf/core/document/object/helpers/stream.hpp"

namespace ripper::pdf::core
{

stream::stream(std::vector<std::byte> data) noexcept : data_(std::move(data)) {}

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

void stream::write(std::span<std::byte> in)
{
    data_.insert(data_.end(), in.begin(), in.end());
}

} // namespace ripper::pdf::core
