#pragma once

#include <optional>

#include "core/document/object/indirect_reference.hpp"
#include "core/document/object/value.hpp"
#include "core/exceptions/exception.hpp"
#include "core/parser/lexer/pdf_lexer.hpp"

namespace ripper::pdf::core
{
    /// Parse an indirect reference of the form `obj gen R`.
    ///
    /// Consumes exactly three tokens and validates their types.
    [[nodiscard]] indirect_reference parse_indirect_reference(pdf_lexer &lexer);

    /// Parse a single PDF value from the current position of `lexer`.
    ///
    /// Handles all PDF value types: null, boolean, integer, real, name, literal
    /// string, hex string, indirect reference, array, and dictionary.
    ///
    /// Indirect references (`obj gen R`) are detected by peeking ahead two tokens
    /// before consuming, so no tokens are lost if the lookahead does not match.
    ///
    /// On unrecognised tokens the token is consumed and a null value is returned.
    [[nodiscard]] value parse_value(pdf_lexer &lexer);

    /// Parse a `[ ... ]` array from `lexer`.
    ///
    /// Assumes the opening `[` token has already been consumed.
    [[nodiscard]] array parse_array(pdf_lexer &lexer);

    /// Parse a `<< ... >>` dictionary from `lexer`.
    ///
    /// Assumes the opening `<<` token has already been consumed.
    [[nodiscard]] dictionary parse_dictionary(pdf_lexer &lexer);
}
