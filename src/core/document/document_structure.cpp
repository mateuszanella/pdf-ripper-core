#include "core/document/document_structure.hpp"

#include <utility>

namespace ripper::core
{
    document_structure::document_structure(
        cross_reference_table compiled_xref,
        std::vector<cross_reference_table> xref_history,
        class trailer compiled_trailer,
        std::vector<class trailer> trailer_history) noexcept
        : compiled_xref_{std::move(compiled_xref)},
          xref_history_{std::move(xref_history)},
          compiled_trailer_{std::move(compiled_trailer)},
          trailer_history_{std::move(trailer_history)}
    {
    }

    cross_reference_table &document_structure::xref() noexcept
    {
        return compiled_xref_;
    }

    const cross_reference_table &document_structure::xref() const noexcept
    {
        return compiled_xref_;
    }

    std::vector<cross_reference_table> &document_structure::xref_history() noexcept
    {
        return xref_history_;
    }

    const std::vector<cross_reference_table> &document_structure::xref_history() const noexcept
    {
        return xref_history_;
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
