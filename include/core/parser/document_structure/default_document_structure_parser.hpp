#pragma once

#include <expected>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    class default_document_structure_parser : public document_structure_parser
    {
    public:
        explicit default_document_structure_parser(reader &reader);

        [[nodiscard]] std::expected<document_structure_result, parser_error> parse();

    private:
        reader &_reader;

        std::unique_ptr<class cross_reference_table_parser> _xref_parser;
        std::unique_ptr<class trailer_parser> _trailer_parser;

        [[nodiscard]] std::expected<std::size_t, parser_error> find_start_xref_offset();
        [[nodiscard]] std::expected<std::size_t, parser_error> extract_prev_offset(const trailer &trailer);
    };
}
