#pragma once

#include "ripper/pdf/core/document/object/indirect_reference.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"

namespace ripper::pdf::core
{
/// Parse an indirect reference of the form `obj gen R`.
///
/// Consumes exactly three tokens and validates their types.
[[nodiscard]] indirect_reference parse_indirect_reference(pdf_lexer& lexer);

/// Parse a single PDF object from the current position of `lexer`.
///
/// Handles all PDF object types: null, boolean, integer, real, name, literal
/// string, hex string, indirect reference, array, and dictionary.
///
/// Indirect references (`obj gen R`) are detected by peeking ahead two tokens
/// before consuming, so no tokens are lost if the lookahead does not match.
///
/// On unrecognised tokens the token is consumed and a null object is returned.
[[nodiscard]] object parse_value(pdf_lexer& lexer);

/// Parse a `[ ... ]` array from `lexer`.
///
/// Assumes the opening `[` token has already been consumed.
[[nodiscard]] array parse_array(pdf_lexer& lexer);

/// Parse a `<< ... >>` dictionary from `lexer`.
///
/// Assumes the opening `<<` token has already been consumed.
[[nodiscard]] dictionary parse_dictionary(pdf_lexer& lexer);

/// Extract the PDF value content from an indirect object's text representation.
///
/// Uses a lexer to locate the `obj` and `endobj` keyword tokens, avoiding
/// false matches on raw byte patterns that appear inside literal strings,
/// hex strings, or other tokenised regions.
///
/// @param content Raw text of an indirect object ("N G obj ... endobj").
/// @return The content between the `obj` and `endobj` keywords.
/// @throws parse_exception if `obj` or `endobj` keywords are not found.
[[nodiscard]] std::string_view extract_object_body(std::string_view content);
} // namespace ripper::pdf::core
