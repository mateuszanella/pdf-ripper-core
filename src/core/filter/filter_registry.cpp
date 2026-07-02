#include "ripper/pdf/core/filter/filter_registry.hpp"

#include "ripper/pdf/core/filter/flate_decode_filter.hpp"

namespace ripper::pdf::core
{

void filter_registry::register_filter(std::string name, std::unique_ptr<stream_filter> impl)
{
    filters_[std::move(name)] = std::move(impl);
}

const stream_filter* filter_registry::get(std::string_view name) const
{
    const auto it = filters_.find(std::string{name});
    if (it != filters_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

filter_registry filter_registry::create_default()
{
    filter_registry registry;
    registry.register_filter("FlateDecode", std::make_unique<flate_decode_filter>());
    return registry;
}

} // namespace ripper::pdf::core
