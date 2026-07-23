#include "ripper/pdf/core/document/revision.hpp"

#include <utility>

namespace ripper::pdf::core
{
revision::revision(cross_reference_section section, class trailer t) noexcept
    : section_{std::move(section)}, trailer_{std::move(t)}
{
}

cross_reference_section& revision::section() noexcept
{
    return section_;
}

const cross_reference_section& revision::section() const noexcept
{
    return section_;
}

class trailer& revision::trailer() noexcept
{
    return trailer_;
}

const class trailer& revision::trailer() const noexcept
{
    return trailer_;
}
} // namespace ripper::pdf::core
