#include "core/document/document_structure.hpp"

#include <utility>

namespace ripper::pdf::core
{
    document_structure::document_structure(
        cross_reference_manager xref_manager,
        class trailer compiled_trailer,
        std::vector<class trailer> trailer_history) noexcept
        : xref_manager_{std::move(xref_manager)},
          compiled_trailer_{std::move(compiled_trailer)},
          trailer_history_{std::move(trailer_history)}
    {
    }

    cross_reference_manager &document_structure::xref() noexcept
    {
        return xref_manager_;
    }

    const cross_reference_manager &document_structure::xref() const noexcept
    {
        return xref_manager_;
    }

    class trailer &document_structure::trailer() noexcept
    {
        return compiled_trailer_;
    }

    const class trailer &document_structure::trailer() const noexcept
    {
        return compiled_trailer_;
    }

    std::vector<class trailer> &document_structure::trailer_history() noexcept
    {
        return trailer_history_;
    }

    const std::vector<class trailer> &document_structure::trailer_history() const noexcept
    {
        return trailer_history_;
    }
}
