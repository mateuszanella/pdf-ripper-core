#include "core/parser/parser.hpp"

#include <functional>
#include <utility>

#include "core/document.hpp"
#include "core/document/catalog/entities/pages.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/catalog/default_catalog_resolver.hpp"

namespace ripper::core
{
    parser::parser(const document &doc, reader &reader)
        : document_{doc}, reader_{reader}
    {
    }

    std::expected<void, parser_error> parser::parse_header_if_needed()
    {
        if (header_.has_value())
            return {};

        header_parser hp{reader_};

        auto result = hp.parse();

        if (!result)
        {
            return std::unexpected(result.error());
        }

        header_ = std::move(*result);

        return {};
    }

    std::expected<void, parser_error> parser::parse_structure_if_needed()
    {
        if (structure_parsed_)
        {
            return {};
        }

        document_structure_parser sp{reader_};

        auto result = sp.parse();

        if (!result)
        {
            return std::unexpected(result.error());
        }

        xref_table_ = std::move(result->compiledXrefTable);
        xref_history_ = std::move(result->xrefTableHistory);
        trailer_ = std::move(result->compiledTrailer);
        trailer_history_ = std::move(result->trailerHistory);
        structure_parsed_ = true;

        return {};
    }

    std::expected<void, parser_error> parser::ensure_structure()
    {
        auto hr = parse_header_if_needed();

        if (!hr)
        {
            return hr;
        }

        return parse_structure_if_needed();
    }

    std::expected<header, parser_error> parser::header()
    {
        auto r = parse_header_if_needed();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        return std::cref(*header_);
    }

    std::expected<cross_reference_table, parser_error> parser::cross_reference_table()
    {
        auto r = parse_structure_if_needed();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        return std::cref(*xref_table_);
    }

    std::expected<trailer, parser_error> parser::trailer()
    {
        auto r = parse_structure_if_needed();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        return std::cref(*trailer_);
    }

    std::expected<catalog, parser_error> parser::catalog()
    {
        auto r = ensure_structure();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        if (catalog_.has_value())
        {
            return std::cref(*catalog_);
        }

        default_catalog_resolver resolver{};

        auto result = resolver.parse(reader_, *xref_table_, *trailer_);

        if (!result)
        {
            return std::unexpected(result.error());
        }

        const auto entry = xref_table_->find(result->catalog_ref);

        if (!entry.has_value() || !entry->in_use())
        {
            return std::unexpected(parser_error::object_not_found);
        }

        catalog_.emplace(document_, result->catalog_ref, entry->offset(), std::nullopt);

        return std::cref(catalog_.value());
    }
}
