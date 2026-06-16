#pragma once

#include "ripper/io/core/reader/reader.hpp"
#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "ripper/pdf/core/parser/document_structure/document_structure_parser.hpp"
#include "ripper/pdf/core/parser/trailer/trailer_parser.hpp"

#include <memory>
#include <optional>

namespace ripper::pdf::core
{
class default_document_structure_parser : public document_structure_parser
{
public:
    explicit default_document_structure_parser(document& document);

    default_document_structure_parser(
        document& document, std::unique_ptr<class cross_reference_table_parser> xref_parser,
        std::unique_ptr<class trailer_parser> trailer_parser);

    [[nodiscard]] document_structure parse();

private:
    document& _document;

    std::unique_ptr<class cross_reference_table_parser> _xref_parser;
    std::unique_ptr<class trailer_parser> _trailer_parser;

    [[nodiscard]] std::optional<std::size_t>
    find_start_xref_offset(ripper::io::core::reader& reader);
    [[nodiscard]] std::optional<std::size_t> extract_prev_offset(const class trailer& trailer);
};
} // namespace ripper::pdf::core
