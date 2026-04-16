#pragma once

#include <expected>
#include <memory>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class default_document_structure_parser : public document_structure_parser
    {
    public:
        explicit default_document_structure_parser(reader &reader);
        default_document_structure_parser(
            reader &reader,
            std::unique_ptr<class cross_reference_table_parser> xref_parser,
            std::unique_ptr<class trailer_parser> trailer_parser);

        [[nodiscard]] std::expected<document_structure_result, error> parse();

    private:
        reader &_reader;

        std::unique_ptr<class cross_reference_table_parser> _xref_parser;
        std::unique_ptr<class trailer_parser> _trailer_parser;

        [[nodiscard]] std::expected<std::size_t, error> find_start_xref_offset();
        [[nodiscard]] std::expected<std::size_t, error> extract_prev_offset(const trailer &trailer);
    };
}
