#include "ripper/pdf/core/document/revision_history.hpp"

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"

#include <utility>

namespace ripper::pdf::core
{
revision_history::revision_history(std::vector<revision> revisions)
    : revisions_{std::move(revisions)}, xref_view_{revisions_}, trailer_view_{revisions_}
{
}

std::vector<revision>& revision_history::revisions() noexcept
{
    return revisions_;
}

const std::vector<revision>& revision_history::revisions() const noexcept
{
    return revisions_;
}

revision& revision_history::active_revision()
{
    return revisions_.back();
}

const revision& revision_history::active_revision() const noexcept
{
    return revisions_.back();
}

void revision_history::push_revision(class revision r)
{
    revisions_.push_back(std::move(r));
}

cross_reference_manager& revision_history::xref() noexcept
{
    return xref_view_;
}

const cross_reference_manager& revision_history::xref() const noexcept
{
    return xref_view_;
}

trailer_manager& revision_history::trailer() noexcept
{
    return trailer_view_;
}

const trailer_manager& revision_history::trailer() const noexcept
{
    return trailer_view_;
}

void revision_history::squash()
{
    auto active = xref_view_.active_entries();

    cross_reference_section consolidated{{}};

    for (auto& [num, ptr] : active)
        consolidated.add_entry(std::move(*ptr));

    auto compiled_trailer = trailer_view_.compiled();
    compiled_trailer.dictionary().remove("Prev");

    revisions_.clear();
    revisions_.emplace_back(std::move(consolidated), std::move(compiled_trailer));
}

std::size_t revision_history::size() const noexcept
{
    return revisions_.size();
}
} // namespace ripper::pdf::core
