#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "core/document.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"
#include "core/document.hpp"

namespace ripper::pdf::core
{
    class default_document_structure_parser : public document_structure_parser
    {
    public:
        explicit default_document_structure_parser(document &document);

        default_document_structure_parser(
            document &document,
            std::unique_ptr<class cross_reference_table_parser> xref_parser,
            std::unique_ptr<class trailer_parser> trailer_parser);

        [[nodiscard]] document_structure parse();

    private:
        document &_document;

        std::unique_ptr<class cross_reference_table_parser> _xref_parser;
        std::unique_ptr<class trailer_parser> _trailer_parser;

        [[nodiscard]] std::optional<std::size_t> find_start_xref_offset(class reader &reader);
        [[nodiscard]] std::optional<std::size_t> extract_prev_offset(const class trailer &trailer);
    };
}
