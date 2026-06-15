#include "core/document/document_structure.hpp"

#include <utility>

namespace ripper::pdf::core
{
document_structure::document_structure(cross_reference_manager xref_manager,
                                       class trailer_manager trailer_manager) noexcept
    : xref_manager_{std::move(xref_manager)}, trailer_manager_{std::move(trailer_manager)}
{
}

cross_reference_manager& document_structure::xref() noexcept
{
    return xref_manager_;
}

const cross_reference_manager& document_structure::xref() const noexcept
{
    return xref_manager_;
}

trailer_manager& document_structure::trailer() noexcept
{
    return trailer_manager_;
}

const trailer_manager& document_structure::trailer() const noexcept
{
    return trailer_manager_;
}
} // namespace ripper::pdf::core
