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
#include "core/document/trailer/trailer_manager.hpp"
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
        auto root_ref = doc_.trailer().compiled().root();
        if (!root_ref)
            throw parse_exception{"Trailer is missing required /Root reference"};

        return ripper::pdf::core::catalog{*doc_.resolve_object(*root_ref)};
    }

    catalog object_factory::create_catalog()
    {
        auto &xref = doc_.cross_reference_table();
        auto &trl = doc_.trailer().active_trailer();

        // Reserve both references up front so the catalog can reference pages
        // by indirect reference from the start.
        auto catalog_ref = xref.reserve();
        auto pages_ref = xref.reserve();

        // Build the root /Pages node: an empty page tree with zero pages.
        dictionary pages_dict;
        pages_dict.set("Type", object{name{"Pages"}});
        pages_dict.set("Kids", object{array{}});
        pages_dict.set("Count", object{std::int64_t{0}});

        auto pages_obj = std::make_unique<indirect_object>(
            object_identity{&doc_, pages_ref}, object{std::move(pages_dict)});

        auto *raw_pages = xref.commit(pages_ref, std::move(pages_obj));
        if (!raw_pages)
            throw logic_exception{"Failed to commit root Pages object to cross-reference table"};

        // Build the /Catalog pointing at the new /Pages node.
        dictionary catalog_dict;
        catalog_dict.set("Type", object{name{"Catalog"}});
        catalog_dict.set("Pages", object{pages_ref});

        auto catalog_obj = std::make_unique<indirect_object>(
            object_identity{&doc_, catalog_ref}, object{std::move(catalog_dict)});

        auto *raw_catalog = xref.commit(catalog_ref, std::move(catalog_obj));
        if (!raw_catalog)
            throw logic_exception{"Failed to commit Catalog (Root) object to cross-reference table"};

        trl.dictionary().set("Root", object{catalog_ref});

        return ripper::pdf::core::catalog{*raw_catalog};
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
        using trailer_manager_t = ripper::pdf::core::trailer_manager;

        // Build the initial section with a single subsection containing object 0 (free-list head)
        cross_reference_subsection::entry_map entries;
        entries.emplace(0, entry_t{iref_t{0, 65535}, 0, false});

        std::vector<cross_reference_subsection> subsections;
        subsections.emplace_back(0, std::move(entries));

        std::vector<cross_reference_section> sections;
        sections.emplace_back(std::move(subsections));

        cross_reference_manager xref_manager{std::move(sections)};

        trailer_t initial_trailer{dictionary{}};
        trailer_manager_t trailer_mgr{std::vector<trailer_t>{std::move(initial_trailer)}};

        return document_structure{
            std::move(xref_manager),
            std::move(trailer_mgr)};
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
