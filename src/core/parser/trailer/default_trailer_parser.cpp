#include "ripper/pdf/core/parser/trailer/default_trailer_parser.hpp"

#include "ripper/pdf/core/document/trailer/trailer.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/lexer/pdf_lexer.hpp"
#include "ripper/pdf/core/parser/value_parsing.hpp"

#include <string_view>

namespace ripper::pdf::core
{
trailer default_trailer_parser::parse_trailer_dict(std::string_view content)
{
    pdf_lexer lexer{content};

    // Scan forward until we find the opening << of the trailer dictionary.
    // Anything before it (e.g. whitespace, comments) is silently skipped.
    while (true)
    {
        const auto token = lexer.next();
        if (token.type == lexer_token_type::eof)
            throw parse_exception{"Trailer dictionary_object was not found"};

        if (token.type == lexer_token_type::dictionary_begin)
            break;
    }

    auto dict = parse_dictionary(lexer);

    return trailer{std::move(dict)};
}

trailer default_trailer_parser::parse(std::string_view content)
{
    // The trailer section always begins with the literal keyword "trailer",
    // followed by the dictionary. Locate it first before handing off to
    // the dictionary_object parser.
    const std::size_t trailerPos = content.find("trailer");
    if (trailerPos == std::string_view::npos)
    {
        throw parse_exception{"Missing trailer keyword"};
    }

    content = content.substr(trailerPos + 7); // skip "trailer"

    return parse_trailer_dict(content);
}
} // namespace ripper::pdf::core
