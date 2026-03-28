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
#include "core/parser/indirect_object/resolver.hpp"

namespace ripper::core
{
    class default_catalog_resolver
    {
    public:
        /**
         * @todo this is shit
         *
         * thsi class should not exist, it should just be part of the
         * document_catalog_parser. must check the impl on content_reader
         * and all of this to just receive the document reference or
         * something that allows access to the traler and xref, and that should
         * be it.
         */
        [[nodiscard]] std::expected<catalog, parser_error> parse(
            const document &doc,
        {
            resolver object_resolver{context};
            content_reader object_content_reader{};
            const default_catalog_parser catalog_object_parser{};

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

                    auto parsed_ref = catalog_object_parser.parse(*content_result, catalog_ref);
                    if (!parsed_ref)
                    {
                        return std::unexpected(parsed_ref.error());
                    }

                    return catalog{doc, *parsed_ref, offset};
                });
        }
    };
}
