#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "core/document/header.hpp"
#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/document/catalog/catalog.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"
#include "core/parser/catalog/catalog_parser.hpp"
#include "core/parser/catalog/pages/pages_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
#include "core/parser/parser_manager.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class document;
    class header_parser;
    class document_structure_parser;
    class indirect_object_resolver;

    struct parsed_structure
    {
        class cross_reference_table compiled_xref;
        std::vector<class cross_reference_table> xref_history;
        class trailer compiled_trailer;
        std::vector<class trailer> trailer_history;
    };

    class parser
    {
    public:
        explicit parser(const document &doc);
        ~parser();

        [[nodiscard]] class parser_manager &manager();

        [[nodiscard]] std::expected<header, parser_error> header();
        [[nodiscard]] std::expected<parsed_structure, parser_error> structure();
        [[nodiscard]] std::expected<catalog, parser_error> catalog();
        [[nodiscard]] std::expected<pages, parser_error> pages(indirect_reference pages_ref);

    private:
        const document &document_;
        std::unique_ptr<class parser_manager> manager_;
    };
}
