#pragma once

#include <expected>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/errors/parser/parser_error.hpp"
#include "core/reader/reader.hpp"

namespace ripper::core
{
    struct document_structure_result
    {
        cross_reference_table compiled_xref;
        std::vector<cross_reference_table> xref_history;

        trailer compiled_trailer;
        std::vector<trailer> trailer_history;
    };

    /**
     * @brief Orchestrates parsing of pdf document structure (xref tables and trailers).
     *
     * Performs a single sweep through the document:
     * 1. Finds startxref offset from end of file
     * 2. Parses xref table at that offset
     * 3. Parses trailer following the xref
     * 4. Follows /prev chain to find older xref/trailer pairs
     * 5. Compiles final merged view with proper override semantics
     */
    class document_structure_parser
    {
    public:
        explicit document_structure_parser(reader &reader);

        [[nodiscard]] std::expected<document_structure_result, parser_error> parse();

    private:
        reader &_reader;

        [[nodiscard]] std::expected<std::size_t, parser_error> find_start_xref_offset();
        [[nodiscard]] std::expected<std::size_t, parser_error> extract_prev_offset(const trailer &trailer);
    };
}
