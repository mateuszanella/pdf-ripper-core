#pragma once

#include "ripper/pdf/core/document/cross_reference_table/cross_reference_section.hpp"
#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/document/trailer/trailer.hpp"

#include <string_view>
#include <utility>

namespace ripper::pdf::core
{
class document;

/// Parses a compressed cross-reference stream (PDF 1.5+ /Type /XRef).
///
/// Delegates dictionary_object parsing to the document's `object_parser`, then extracts
/// the binary xref stream data from the decoded stream payload.
///
/// Returns both the cross-reference section and the trailer, since xref streams
/// embed trailer metadata in the stream dictionary.
///
/// @see PDF spec §7.5.8
class compressed_cross_reference_table_parser
{
public:
    /// Parse a compressed xref stream from raw content.
    ///
    /// `content` is the raw bytes of the indirect object containing the xref
    /// stream (from `N G obj` to `endobj`).
    ///
    /// `temp_ref` is a temporary indirect reference used to identify the xref
    /// stream object during parsing (e.g., `indirect_reference{0, 0}`).
    ///
    /// Delegates to `doc.parser()->manager().object_parser()` for proper
    /// dictionary_object parsing instead of manual string manipulation.
    ///
    /// @throws parse_exception if the content is malformed or required keys are missing.
    [[nodiscard]] static std::pair<cross_reference_section, trailer>
    parse(document& doc, std::string_view content, indirect_reference temp_ref);
};

} // namespace ripper::pdf::core
