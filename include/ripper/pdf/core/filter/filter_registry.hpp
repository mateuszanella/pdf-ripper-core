#pragma once

#include "ripper/pdf/core/filter/stream_filter.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ripper::pdf::core
{

class filter_registry
{
public:
    void register_filter(std::string name, std::unique_ptr<stream_filter> impl);

    [[nodiscard]] const stream_filter* get(std::string_view name) const;

    [[nodiscard]] static filter_registry create_default();

private:
    std::unordered_map<std::string, std::unique_ptr<stream_filter>> filters_;
};

} // namespace ripper::pdf::core
