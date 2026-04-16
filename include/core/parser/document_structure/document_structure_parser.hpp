#pragma once

#include <expected>
#include <vector>

#include "core/document/cross_reference_table/cross_reference_table.hpp"
#include "core/document/trailer/trailer.hpp"
#include "core/error.hpp"
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

    class document_structure_parser
    {
    public:
        virtual ~document_structure_parser() = default;
        virtual std::expected<document_structure_result, error> parse() = 0;
    };
}
