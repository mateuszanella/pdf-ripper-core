#include "core/document/object_factory.hpp"

#include <utility>

#include "core/document.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/document/cross_reference_table/cross_reference_entry.hpp"
#include "core/document/cross_reference_table/cross_reference_manager.hpp"
#include "core/document/cross_reference_table/cross_reference_section.hpp"
#include "core/document/cross_reference_table/cross_reference_subsection.hpp"
#include "core/document/document_structure.hpp"
#include "core/document/header.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/document/object/object.hpp"
#include "core/document/object/indirect_object.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/parser.hpp"

namespace ripper::pdf::core
{
    object_factory::object_factory(document &doc) noexcept
        : doc_(doc)
    {
    }

    catalog object_factory::parse_catalog()
    {
        auto root_ref = doc_.trailer().root();
        if (!root_ref)
            throw parse_exception{"Trailer is missing required /Root reference"};

        return ripper::pdf::core::catalog{*doc_.resolve_object(*root_ref)};
    }

    catalog object_factory::create_catalog()
    {
        auto &xref = doc_.cross_reference_table();
        auto &trl = doc_.trailer();

        auto ref = xref.reserve();

        dictionary dict;
        dict.set("Type", object{name{"Catalog"}});

        auto obj = std::make_unique<indirect_object>(object_identity{&doc_, ref}, object{std::move(dict)});

        auto *raw = xref.commit(ref, std::move(obj));
        if (!raw)
            throw logic_exception{"Failed to commit catalog to cross-reference table"};

        trl.dictionary().set("Root", object{ref});

        return ripper::pdf::core::catalog{*raw};
    }

    document_structure object_factory::parse_structure() const
    {
        if (!doc_.has_parser())
            throw logic_exception{"No parser available"};

        return doc_.parser()->structure();
    }

    document_structure object_factory::create_structure() const
    {
        using entry_t = ripper::pdf::core::cross_reference_entry;
        using iref_t = ripper::pdf::core::indirect_reference;
        using trailer_t = ripper::pdf::core::trailer;

        // Build the initial section with a single subsection containing object 0 (free-list head)
        cross_reference_subsection::entry_map entries;
        entries.emplace(0, entry_t{iref_t{0, 65535}, 0, false});

        std::vector<cross_reference_subsection> subsections;
        subsections.emplace_back(0, std::move(entries));

        std::vector<cross_reference_section> sections;
        sections.emplace_back(std::move(subsections));

        cross_reference_manager xref_manager{std::move(sections)};

        trailer_t initial_trailer{dictionary{}};
        std::vector<trailer_t> trailer_history;
        trailer_history.push_back(initial_trailer);

        return document_structure{
            std::move(xref_manager),
            initial_trailer,
            std::move(trailer_history)};
    }

    header object_factory::parse_header() const
    {
        if (!doc_.has_parser())
            throw logic_exception{"No parser available"};

        return doc_.parser()->header();
    }

    header object_factory::create_header() const
    {
        return ripper::pdf::core::header{"1.4"};
    }
}
