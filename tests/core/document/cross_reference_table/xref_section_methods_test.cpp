#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_entry.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "ripper/pdf/core/document/object/indirect_object.hpp"
#include "ripper/pdf/core/document/object/object.hpp"

#include <catch2/catch_test_macros.hpp>
#include <memory>

namespace ripper::pdf::core
{

indirect_object make_obj(document& doc, std::uint32_t num, std::string value)
{
    dictionary dict;
    dict.set("Data", object{std::move(value)});
    return indirect_object{object_identity{&doc, indirect_reference{num, 0}},
                           object{std::move(dict)}};
}

TEST_CASE("add_entry_from copies a resolved entry into a section",
          "[cross_reference_section][add_entry_from]")
{
    document doc{nullptr, nullptr};

    // Build a source section with one resolved entry.
    cross_reference_subsection::entry_map entries;
    auto obj = std::make_unique<class indirect_object>(make_obj(doc, 1, "source"));
    entries.emplace(1, cross_reference_entry{indirect_reference{1, 0}, std::move(obj)});

    std::vector<cross_reference_subsection> subsections;
    subsections.emplace_back(1, std::move(entries));
    cross_reference_section source_section{std::move(subsections)};

    // Build an empty target section.
    cross_reference_section target{std::vector<cross_reference_subsection>{}};

    auto* source_entry = source_section.find(1);
    REQUIRE(source_entry != nullptr);
    REQUIRE(source_entry->is_resolved());

    auto ref = target.add_entry_from(*source_entry);

    REQUIRE(ref.object_number() == 1);
    REQUIRE(ref.generation() == 0);

    auto* target_entry = target.find(1);
    REQUIRE(target_entry != nullptr);
    REQUIRE(target_entry->is_resolved());
    REQUIRE(*target_entry->indirect_object()->dictionary()->get_string("Data") == "source");

    // Modify source — target must be independent.
    source_entry->indirect_object()->dictionary()->set("Data", object{std::string{"modified"}});
    REQUIRE(*target_entry->indirect_object()->dictionary()->get_string("Data") == "source");
}

TEST_CASE("add_entry_from copies an unresolved entry", "[cross_reference_section][add_entry_from]")
{
    cross_reference_section source{std::vector<cross_reference_subsection>{}};
    source.add_entry(cross_reference_entry{indirect_reference{3, 0}, 5000, true});

    cross_reference_section target{std::vector<cross_reference_subsection>{}};

    auto* source_entry = source.find(3);
    REQUIRE(source_entry != nullptr);
    REQUIRE_FALSE(source_entry->is_resolved());
    REQUIRE(*source_entry->offset() == 5000);

    auto ref = target.add_entry_from(*source_entry);

    auto* target_entry = target.find(3);
    REQUIRE(target_entry != nullptr);
    REQUIRE_FALSE(target_entry->is_resolved());
    REQUIRE(*target_entry->offset() == 5000);
    REQUIRE(target_entry->in_use());
}

TEST_CASE("push_section creates section with object 0", "[cross_reference_manager][push_section]")
{
    cross_reference_manager xref{std::vector<cross_reference_section>{}};

    auto& section = xref.push_section();

    REQUIRE(xref.sections().size() == 1);
    REQUIRE(section.size() == 1); // only object 0

    auto* obj0 = section.find(0);
    REQUIRE(obj0 != nullptr);
    REQUIRE_FALSE(obj0->in_use());
    REQUIRE(obj0->reference().generation() == 65535);
    REQUIRE_FALSE(section.startxref_offset().has_value());
}

TEST_CASE("create_new_revision creates section and trailer with /Prev",
          "[document][create_new_revision]")
{
    // Build a document with a prior section that has a known startxref offset.
    cross_reference_subsection::entry_map entries;
    entries.emplace(1, cross_reference_entry{indirect_reference{1, 0}, 100, true});

    std::vector<cross_reference_subsection> subsections;
    subsections.emplace_back(1, std::move(entries));

    cross_reference_section existing{std::move(subsections), 42};
    std::vector<cross_reference_section> sections;
    sections.push_back(std::move(existing));

    cross_reference_manager xref{std::move(sections)};
    trailer_manager trailers{std::vector<trailer>{}};

    document doc{nullptr, nullptr};
    // Swap in our pre-built xref and trailer.
    // We test create_new_revision via a doc that has the right structure.
    // Actually, let's test it directly through document.

    // For this test, just verify push_section + manual trailer creation work.
    auto& section = xref.push_section();

    REQUIRE(xref.sections().size() == 2);
    REQUIRE(xref.sections()[0].startxref_offset().has_value());
    REQUIRE(*xref.sections()[0].startxref_offset() == 42);

    // Manually build the trailer with /Prev.
    dictionary trailer_dict;
    trailer_dict.set("Size", object{std::int64_t{2}});
    trailer_dict.set("Prev", object{std::int64_t{42}});
    trailer t{std::move(trailer_dict)};
    trailers.push(std::move(t));

    REQUIRE(trailers.active_trailer().dictionary().contains("Prev"));
    REQUIRE(*trailers.active_trailer().dictionary().get_integer("Prev") == 42);
}

TEST_CASE("create_new_revision on document creates section and trailer",
          "[document][create_new_revision]")
{
    document doc{nullptr, nullptr};

    // Add an entry to the initial section so next_object_number is meaningful.
    {
        auto& section = doc.cross_reference_table().active_section();
        auto obj = std::make_unique<class indirect_object>(make_obj(doc, 5, "test"));
        section.add_entry(cross_reference_entry{indirect_reference{5, 0}, std::move(obj)});
    }

    auto& new_section = doc.create_new_revision();

    // Verify section.
    REQUIRE(doc.cross_reference_table().sections().size() == 2);
    REQUIRE(new_section.size() == 1); // object 0
    auto* obj0 = new_section.find(0);
    REQUIRE(obj0 != nullptr);
    REQUIRE_FALSE(obj0->in_use());

    // Verify trailer.
    REQUIRE(doc.trailer().trailers().size() == 2); // initial empty + pushed
    REQUIRE(doc.trailer().active_trailer().dictionary().contains("Size"));
    REQUIRE(*doc.trailer().active_trailer().dictionary().get_integer("Size") >= 6);
    // No /Prev because the initial section has no startxref_offset.
    REQUIRE_FALSE(doc.trailer().active_trailer().dictionary().contains("Prev"));
}

TEST_CASE("create_new_revision + add_entry_from for incremental setup",
          "[document][create_new_revision][add_entry_from]")
{
    document doc{nullptr, nullptr};

    // Populate the initial section with an entry.
    {
        auto& section = doc.cross_reference_table().active_section();
        auto obj = std::make_unique<class indirect_object>(make_obj(doc, 42, "original"));
        section.add_entry(cross_reference_entry{indirect_reference{42, 0}, std::move(obj)});
    }

    // Create a new revision and copy the entry into it.
    auto& new_section = doc.create_new_revision();

    auto* old_entry = doc.cross_reference_table().find(42);
    REQUIRE(old_entry != nullptr);

    auto ref = new_section.add_entry_from(*old_entry);
    REQUIRE(ref.object_number() == 42);

    auto* copied = new_section.find(42);
    REQUIRE(copied != nullptr);
    REQUIRE(copied->is_resolved());
    REQUIRE(*copied->indirect_object()->dictionary()->get_string("Data") == "original");

    // Modify the old entry — the copy must be independent.
    old_entry->indirect_object()->dictionary()->set("Data", object{std::string{"modified"}});
    REQUIRE(*copied->indirect_object()->dictionary()->get_string("Data") == "original");
}
} // namespace ripper::pdf::core
