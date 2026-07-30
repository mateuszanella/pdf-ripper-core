#include "ripper/pdf/core/filter/filter_manager.hpp"

#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/filter/flate_decode_filter.hpp"

#include <unordered_map>

namespace ripper::pdf::core
{
namespace
{

using filter_chain_t = std::vector<const stream_filter*>;

filter_chain_t build_chain(const dictionary_object& dict)
{
    const auto* filter_entry = dict.get("Filter");
    if (filter_entry == nullptr)
    {
        return {};
    }

    filter_chain_t chain;

    const auto* filter_name = filter_entry->as_name();
    if (filter_name != nullptr)
    {
        const auto* filter = filter_manager::get(filter_name->value);
        if (filter == nullptr)
        {
            throw parse_exception{std::string{"Unknown filter: "} + filter_name->value};
        }
        chain.push_back(filter);
        return chain;
    }

    const auto* filter_array = filter_entry->as_array();
    if (filter_array == nullptr)
    {
        throw parse_exception{"/Filter must be a name or an array of names"};
    }

    chain.reserve(filter_array->size());
    for (const auto& entry : *filter_array)
    {
        const auto* name = entry.as_name();
        if (name == nullptr)
        {
            throw parse_exception{"Filter array entry is not a name"};
        }
        const auto* filter = filter_manager::get(name->value);
        if (filter == nullptr)
        {
            throw parse_exception{std::string{"Unknown filter: "} + name->value};
        }
        chain.push_back(filter);
    }

    return chain;
}

} // namespace

std::unordered_map<std::string, std::unique_ptr<stream_filter>>& filter_manager::filters()
{
    static std::unordered_map<std::string, std::unique_ptr<stream_filter>> instance;
    return instance;
}

void filter_manager::ensure_defaults()
{
    static bool initialized = []
    {
        register_filter("FlateDecode", std::make_unique<flate_decode_filter>());
        return true;
    }();
    (void)initialized;
}

void filter_manager::register_filter(std::string name, std::unique_ptr<stream_filter> impl)
{
    filters()[std::move(name)] = std::move(impl);
}

const stream_filter* filter_manager::get(std::string_view name)
{
    ensure_defaults();
    const auto it = filters().find(std::string{name});
    return it != filters().end() ? it->second.get() : nullptr;
}

bool filter_manager::has(std::string_view name)
{
    return get(name) != nullptr;
}

void filter_manager::forget(std::string_view name)
{
    filters().erase(std::string{name});
}

std::vector<std::byte> filter_manager::decode(const dictionary_object& dict,
                                              std::span<const std::byte> raw)
{
    const auto chain = build_chain(dict);
    if (chain.empty())
    {
        return {raw.begin(), raw.end()};
    }

    auto result = chain[0]->decode(raw);
    for (std::size_t i = 1; i < chain.size(); ++i)
    {
        result = chain[i]->decode(result);
    }
    return result;
}

std::vector<std::byte> filter_manager::encode(const dictionary_object& dict,
                                              std::span<const std::byte> decoded)
{
    const auto chain = build_chain(dict);
    if (chain.empty())
    {
        return {decoded.begin(), decoded.end()};
    }

    auto result = chain.back()->encode(decoded);
    for (auto it = chain.rbegin() + 1; it != chain.rend(); ++it)
    {
        result = (*it)->encode(result);
    }
    return result;
}

} // namespace ripper::pdf::core
