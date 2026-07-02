#include "ripper/pdf/core/filter/filter_chain.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"

namespace ripper::pdf::core
{

filter_chain::filter_chain(const dictionary& stream_dict)
    : registry_(filter_registry::create_default())
{
    const auto* filter_entry = stream_dict.get("Filter");
    if (filter_entry == nullptr)
    {
        return;
    }

    const auto* filter_name = filter_entry->as_name();
    if (filter_name != nullptr)
    {
        const auto* filter = registry_.get(filter_name->value);
        if (filter == nullptr)
        {
            throw parse_exception{std::string{"Unknown filter: "} + filter_name->value};
        }
        filters_.push_back(filter);
        return;
    }

    const auto* filter_array = filter_entry->as_array();
    if (filter_array != nullptr)
    {
        filters_.reserve(filter_array->size());
        for (const auto& entry : *filter_array)
        {
            const auto* name = entry.as_name();
            if (name == nullptr)
            {
                throw parse_exception{"Filter array entry is not a name"};
            }
            const auto* filter = registry_.get(name->value);
            if (filter == nullptr)
            {
                throw parse_exception{std::string{"Unknown filter: "} + name->value};
            }
            filters_.push_back(filter);
        }
        return;
    }

    throw parse_exception{"/Filter must be a name or an array of names"};
}

bool filter_chain::has_filters() const
{
    return !filters_.empty();
}

std::vector<std::byte> filter_chain::decode(std::span<const std::byte> input) const
{
    auto result = std::vector<std::byte>{input.begin(), input.end()};
    for (const auto* filter : filters_)
    {
        result = filter->decode(result);
    }
    return result;
}

std::vector<std::byte> filter_chain::encode(std::span<const std::byte> input) const
{
    auto result = std::vector<std::byte>{input.begin(), input.end()};
    for (auto it = filters_.rbegin(); it != filters_.rend(); ++it)
    {
        result = (*it)->encode(result);
    }
    return result;
}

} // namespace ripper::pdf::core
