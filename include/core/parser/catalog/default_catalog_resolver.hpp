#pragma once

#include <cstdint>
#include <expected>

#include "core/document.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/indirect_reference.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/catalog/default_catalog_parser.hpp"
#include "core/parser/indirect_object/content_reader.hpp"
#include "core/parser/indirect_object/resolution_context.hpp"
#include "core/parser/indirect_object/resolver.hpp"

namespace ripper::core
{
    class default_catalog_resolver
    {
    public:
        [[nodiscard]] std::expected<catalog, parser_error> parse(
            const document &doc,
            const cross_reference_table &xref_table,
            const trailer &trailer_obj) const
        {
            resolution_context context{doc, xref_table, trailer_obj};
            resolver object_resolver{context};
            content_reader object_content_reader{};

            return object_resolver.resolve<catalog>(
                [&](const resolution_context &) -> std::expected<indirect_reference, parser_error>
                {
                    if (!trailer_obj.root().has_value())
                    {
                        return std::unexpected(parser_error::missing_catalog);
                    }

                    return *trailer_obj.root();
                },
                [&](const indirect_reference &catalog_ref, std::uint64_t offset) -> std::expected<catalog, parser_error>
                {
                    auto &doc_reader = doc.reader();
                    auto content_result = object_content_reader.read(doc_reader, offset);
                    if (!content_result)
                    {
                        return std::unexpected(content_result.error());
                    }

                    default_catalog_parser object_parser{};

                    auto parsed_ref = object_parser.parse(*content_result, catalog_ref);
                    if (!parsed_ref)
                    {
                        return std::unexpected(parsed_ref.error());
                    }

                    return catalog{doc, *parsed_ref, offset};
                });
        }
    };
}
