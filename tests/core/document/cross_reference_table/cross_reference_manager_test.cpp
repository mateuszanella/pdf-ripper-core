#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vector>

namespace ripper::pdf::core
{
namespace
{
cross_reference_section make_section(std::vector<cross_reference_entry> entries)
{
    cross_reference_subsection::entry_map map;
    for (auto& entry : entries)
    {
        map.emplace(entry.reference().object_number(), std::move(entry));
    }

    cross_reference_subsection subsection{0, std::move(map)};

    std::vector<cross_reference_subsection> subsections;
    subsections.push_back(std::move(subsection));

    return cross_reference_section{std::move(subsections)};
}
} // namespace

TEST_CASE("cross_reference_manager prefers newest section entries", "[xref][manager]")
{
    std::vector<cross_reference_entry> older_entries;
    older_entries.emplace_back(indirect_reference{1, 0}, 11, true);
    auto older = make_section(std::move(older_entries));

    std::vector<cross_reference_entry> newer_entries;
    newer_entries.emplace_back(indirect_reference{1, 1}, 22, true);
    auto newer = make_section(std::move(newer_entries));

    std::vector<cross_reference_section> sections;
    sections.push_back(std::move(older));
    sections.push_back(std::move(newer));
    cross_reference_manager manager(std::move(sections));

    auto* entry = manager.find(1);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->offset().has_value());
    REQUIRE(*entry->offset() == 22);
    REQUIRE(entry->reference().generation() == 1);
}

TEST_CASE("cross_reference_manager active_entries filters deleted objects", "[xref][manager]")
{
    std::vector<cross_reference_entry> entries;
    entries.emplace_back(indirect_reference{0, 65535}, 0, false);
    entries.emplace_back(indirect_reference{1, 0}, 12, false);
    entries.emplace_back(indirect_reference{2, 0}, 24, true);
    auto section = make_section(std::move(entries));

    std::vector<cross_reference_section> sections;
    sections.push_back(std::move(section));
    cross_reference_manager manager(std::move(sections));
    auto active = manager.active_entries();

    REQUIRE(active.contains(0));
    REQUIRE_FALSE(active.contains(1));
    REQUIRE(active.contains(2));
}

TEST_CASE("cross_reference_manager reserve appends pending entry", "[xref][manager]")
{
    std::vector<cross_reference_section> sections;
    sections.emplace_back(std::vector<cross_reference_subsection>{});
    cross_reference_manager manager(std::move(sections));

    const auto ref = manager.reserve();
    auto* entry = manager.find(ref);

    REQUIRE(ref.object_number() == 1);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->is_new());
    REQUIRE_FALSE(entry->is_resolved());
}
} // namespace ripper::pdf::core
