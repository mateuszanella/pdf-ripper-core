#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/revision.hpp"
#include "ripper/pdf/core/document/revision_manager.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
revision make_revision_from_entries(std::vector<cross_reference_entry> entries,
                                    std::optional<std::uint64_t> startxref = std::nullopt)
{
    cross_reference_subsection::entry_map map;
    for (auto& entry : entries)
    {
        map.emplace(entry.reference().object_number(), std::move(entry));
    }

    cross_reference_subsection subsection{0, std::move(map)};

    std::vector<cross_reference_subsection> subsections;
    subsections.push_back(std::move(subsection));

    cross_reference_section section{std::move(subsections), startxref};
    trailer t{dictionary_object{}};
    return revision{std::move(section), std::move(t)};
}
} // namespace

TEST_CASE("cross_reference_manager prefers newest section entries", "[xref][manager]")
{
    std::vector<cross_reference_entry> older_entries;
    older_entries.emplace_back(indirect_reference{1, 0}, 11, true);

    std::vector<cross_reference_entry> newer_entries;
    newer_entries.emplace_back(indirect_reference{1, 1}, 22, true);

    std::vector<revision> revisions;
    revisions.push_back(make_revision_from_entries(std::move(older_entries)));
    revisions.push_back(make_revision_from_entries(std::move(newer_entries)));

    revision_manager manager{std::move(revisions)};

    auto* entry = manager.xref().find(1);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->offset() == 22);
    REQUIRE(entry->reference().generation() == 1);
}

TEST_CASE("cross_reference_manager active_entries filters deleted objects", "[xref][manager]")
{
    std::vector<cross_reference_entry> entries;
    entries.emplace_back(indirect_reference{0, 65535}, 0, false);
    entries.emplace_back(indirect_reference{1, 0}, 12, false);
    entries.emplace_back(indirect_reference{2, 0}, 24, true);

    std::vector<revision> revisions;
    revisions.push_back(make_revision_from_entries(std::move(entries)));

    revision_manager manager{std::move(revisions)};
    auto active = manager.xref().active_entries();

    REQUIRE(active.contains(0));
    REQUIRE_FALSE(active.contains(1));
    REQUIRE(active.contains(2));
}

TEST_CASE("cross_reference_manager reserve appends pending entry", "[xref][manager]")
{
    cross_reference_subsection::entry_map entries;
    entries.emplace(0, cross_reference_entry{indirect_reference{0, 65535}, 0, false});

    std::vector<cross_reference_subsection> subsections;
    subsections.emplace_back(0, std::move(entries));

    cross_reference_section section{std::move(subsections)};
    trailer t{dictionary_object{}};

    std::vector<revision> revisions;
    revisions.emplace_back(std::move(section), std::move(t));

    revision_manager manager{std::move(revisions)};

    const auto ref = manager.xref().reserve();
    auto* entry = manager.xref().find(ref);

    REQUIRE(ref.object_number() == 1);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->is_new());
    REQUIRE_FALSE(entry->is_resolved());
}
} // namespace ripper::pdf::core
