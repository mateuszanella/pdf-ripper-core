#pragma once

#include <expected>
#include <vector>

#include "Core/Document/CrossReferenceTable/CrossReferenceTable.hpp"
#include "Core/Document/Trailer/Trailer.hpp"
#include "Core/Errors/Parser/ParserError.hpp"
#include "Core/Reader/Reader.hpp"

namespace Ripper::Core
{
    struct DocumentStructureResult
    {
        CrossReferenceTable compiledXrefTable;
        std::vector<CrossReferenceTable> xrefTableHistory;

        Trailer compiledTrailer;
        std::vector<Trailer> trailerHistory;
    };

    /**
     * @brief Orchestrates parsing of PDF document structure (xref tables and trailers).
     *
     * Performs a single sweep through the document:
     * 1. Finds startxref offset from end of file
     * 2. Parses xref table at that offset
     * 3. Parses trailer following the xref
     * 4. Follows /Prev chain to find older xref/trailer pairs
     * 5. Compiles final merged view with proper override semantics
     */
    class DocumentStructureParser
    {
    public:
        explicit DocumentStructureParser(Reader &reader);

        [[nodiscard]] std::expected<DocumentStructureResult, ParserError> Parse();

    private:
        Reader &_reader;

        [[nodiscard]] std::expected<std::size_t, ParserError> FindStartXrefOffset();
        [[nodiscard]] std::expected<std::size_t, ParserError> ExtractPrevOffset(const Trailer &trailer);
    };
}
