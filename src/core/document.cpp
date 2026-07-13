#include "ripper/pdf/core/document.hpp"

#include "ripper/io/core/reader/file_reader.hpp"
#include "ripper/io/core/reader/reader.hpp"
#include "ripper/io/core/writer/file_writer.hpp"
#include "ripper/io/core/writer/writer.hpp"
#include "ripper/pdf/core/document/catalog/catalog.hpp"
#include "ripper/pdf/core/document/cross_reference_table/cross_reference_manager.hpp"
#include "ripper/pdf/core/document/document_revision.hpp"
#include "ripper/pdf/core/document/header.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/document/trailer/trailer_manager.hpp"
#include "ripper/pdf/core/document_save_strategy/incremental_document_save_strategy.hpp"
#include "ripper/pdf/core/document_save_strategy/linearize_document_save_strategy.hpp"
#include "ripper/pdf/core/document_save_strategy/raw_document_save_strategy.hpp"
#include "ripper/pdf/core/document_save_strategy/save_strategy_type.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/parser.hpp"
#include "ripper/pdf/core/serializer/serializer.hpp"

#include <memory>
#include <utility>

namespace ripper::pdf::core
{
document::document(std::unique_ptr<ripper::io::core::reader> reader,
                   std::unique_ptr<ripper::io::core::writer> writer)
    : reader_(std::move(reader)), writer_(std::move(writer))
{
    if (reader_)
        parser_ = std::make_unique<class parser>(*this);

    if (writer_)
        serializer_ = std::make_unique<class serializer>(*this);
}

document document::open(const std::filesystem::path& path)
{
    return document{std::make_unique<ripper::io::core::file_reader>(path), nullptr};
}

document document::create(const std::filesystem::path& path)
{
    return document{nullptr, std::make_unique<ripper::io::core::file_writer>(path)};
}

void document::save()
{
    if (!save_strategy_)
        save_strategy_ = std::make_unique<linearize_document_save_strategy>();

    save_strategy_->save(*this);
}

void document::save(save_strategy_type type)
{
    switch (type)
    {
        case save_strategy_type::linearize:
            save_strategy_ = std::make_unique<linearize_document_save_strategy>();
            break;
        case save_strategy_type::raw:
            save_strategy_ = std::make_unique<raw_document_save_strategy>();
            break;
        case save_strategy_type::incremental:
            save_strategy_ = std::make_unique<incremental_document_save_strategy>();
            break;
    }

    save();
}

void document::set_save_strategy(std::unique_ptr<class document_save_strategy> strategy)
{
    save_strategy_ = std::move(strategy);
}

bool document::has_reader() const
{
    return static_cast<bool>(reader_);
}

bool document::has_parser() const
{
    return static_cast<bool>(parser_);
}

bool document::has_writer() const
{
    return static_cast<bool>(writer_);
}

bool document::has_serializer() const
{
    return static_cast<bool>(serializer_);
}

ripper::io::core::reader* document::reader() const
{
    return reader_.get();
}

parser* document::parser() const
{
    return parser_.get();
}

ripper::io::core::writer* document::writer() const
{
    return writer_.get();
}

serializer* document::serializer() const
{
    return serializer_.get();
}

header& document::header()
{
    if (header_.has_value())
        return *header_;

    header_ = has_parser() ? object_manager::parse_header(*this) : object_manager::create_header();

    return *header_;
}

cross_reference_manager& document::cross_reference_table()
{
    return revision().xref();
}

trailer_manager& document::trailer()
{
    return revision().trailer();
}

catalog document::catalog()
{
    auto root_ref = trailer().compiled().root();

    if (!root_ref)
        return object_manager::create_catalog(*this);

    auto* entry = cross_reference_table().find(*root_ref);
    if (entry == nullptr)
        throw parse_exception{"Root indirect_object not found in cross-reference table"};

    if (entry->is_resolved())
        return ripper::pdf::core::catalog{*entry->indirect_object()};

    return object_manager::parse_catalog(*this);
}

indirect_object* document::resolve_object(indirect_reference ref)
{
    auto* entry = cross_reference_table().find(ref);
    if (entry == nullptr)
        throw parse_exception{"Object not found in cross-reference table"};

    if (entry->is_resolved())
        return entry->indirect_object();

    if (!has_parser())
        throw logic_exception{"No parser available to resolve unresolved indirect_object"};

    auto parsed = parser_->parse_object(ref);

    return entry->resolve(std::make_unique<indirect_object>(std::move(parsed)));
}

indirect_object& document::resolve_object_to_active_revision(indirect_reference ref)
{
    auto& xref = cross_reference_table();
    auto& active = xref.active_section();

    // Already in the active section, no need to resolve
    if (auto* existing = active.find(ref))
    {
        if (auto* obj = existing->indirect_object())
            return *obj;
    }

    // Find in any section (newest-wins)
    auto* entry = xref.find(ref);
    if (entry == nullptr)
        throw logic_exception{"Object " + std::to_string(ref.object_number()) + " not found"};

    // Ensure the source entry is resolved (lazy-load from file if needed) before cloning
    resolve_object(ref);

    // Clone to active section
    auto* cloned_entry = active.add_entry_from(*entry);
    if (cloned_entry == nullptr)
        throw logic_exception{"Failed to clone object " + std::to_string(ref.object_number()) +
                              " to active revision"};

    auto* obj = cloned_entry->indirect_object();
    if (obj == nullptr)
        throw logic_exception{"Cloned object " + std::to_string(ref.object_number()) +
                              " has no indirect object"};

    return *obj;
}

cross_reference_section& document::create_new_revision()
{
    auto& new_section = cross_reference_table().push_section();
    auto& t = trailer().push_trailer();

    t.dictionary().set(
        "Size", object{static_cast<std::int64_t>(cross_reference_table().next_object_number())});

    // Point /Prev at the previous section's file offset, if any.
    auto& sections = cross_reference_table().sections();
    if (sections.size() > 1)
    {
        auto& prev = sections[sections.size() - 2];
        if (prev.startxref_offset().has_value())
            t.dictionary().set("Prev", object{static_cast<std::int64_t>(*prev.startxref_offset())});
    }

    return new_section;
}

document_revision& document::revision()
{
    if (revision_.has_value())
        return *revision_;

    revision_ =
        has_parser() ? object_manager::parse_revision(*this) : object_manager::create_revision();

    return *revision_;
}

} // namespace ripper::pdf::core
