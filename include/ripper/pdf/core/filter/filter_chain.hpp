#pragma once

#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/filter/filter_registry.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace ripper::pdf::core
{

class filter_chain
{
public:
    explicit filter_chain(const dictionary& stream_dict);

    [[nodiscard]] bool has_filters() const;

    [[nodiscard]] std::vector<std::byte> decode(std::span<const std::byte> input) const;

    [[nodiscard]] std::vector<std::byte> encode(std::span<const std::byte> input) const;

private:
    filter_registry registry_;
    std::vector<const stream_filter*> filters_;
};

} // namespace ripper::pdf::core
