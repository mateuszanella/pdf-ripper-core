#include "ripper/pdf/core/document/document_revision.hpp"

#include <utility>

namespace ripper::pdf::core
{
document_revision::document_revision(cross_reference_manager xref_manager,
                                       class trailer_manager trailer_manager) noexcept
    : xref_manager_{std::move(xref_manager)}, trailer_manager_{std::move(trailer_manager)}
{
}

cross_reference_manager& document_revision::xref() noexcept
{
    return xref_manager_;
}

const cross_reference_manager& document_revision::xref() const noexcept
{
    return xref_manager_;
}

trailer_manager& document_revision::trailer() noexcept
{
    return trailer_manager_;
}

const trailer_manager& document_revision::trailer() const noexcept
{
    return trailer_manager_;
}
} // namespace ripper::pdf::core
