#include "core/parser/parser.hpp"

#include <utility>

#include "core/document.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/catalog/default_catalog_resolver.hpp"

namespace ripper::core
{
    parser::parser(const document &doc)
        : document_{doc}
    {
    }

    std::expected<void, parser_error> parser::parse_header_if_needed()
    {
        if (header_.has_value())
            return {};

        header_parser hp{document_.reader()};

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

        document_structure_parser sp{document_.reader()};

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
        auto err = parse_header_if_needed();

        if (!err)
        {
            return err;
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

        return header_.value();
    }

    std::expected<cross_reference_table, parser_error> parser::cross_reference_table()
    {
        auto r = parse_structure_if_needed();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        return xref_table_.value();
    }

    std::expected<trailer, parser_error> parser::trailer()
    {
        auto r = parse_structure_if_needed();

        if (!r)
        {
            return std::unexpected(r.error());
        }

        return trailer_.value();
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
            return catalog_.value();
        }

        default_catalog_resolver catalog_resolver{};

        auto resolved_catalog = catalog_resolver.parse(document_);

        if (!resolved_catalog)
        {
            return std::unexpected(resolved_catalog.error());
        }

        catalog_.emplace(std::move(*resolved_catalog));

        return catalog_.value();
    }
}
