#include "ripper/pdf/core/parser/cross_reference_table/compressed_cross_reference_table_parser.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/document/object/object.hpp"
#include "ripper/pdf/core/exceptions/exception.hpp"
#include "ripper/pdf/core/parser/cross_reference_table/cross_reference_stream_parser.hpp"
#include "ripper/pdf/core/parser/object_parser.hpp"
#include "ripper/pdf/core/parser/parser.hpp"
#include "ripper/pdf/core/parser/parser_manager.hpp"
#include "ripper/pdf/core/parser/value_parsing.hpp"

namespace ripper::pdf::core
{

std::pair<cross_reference_section, trailer>
compressed_cross_reference_table_parser::parse(document& doc, std::string_view content,
                                               indirect_reference temp_ref)
{
    auto* parser = doc.parser();
    if (parser == nullptr)
        throw logic_exception{"No parser available to parse xref stream"};

    content = extract_object_body(content);

    auto parsed_obj = parser->manager().object_parser().parse(doc, temp_ref, content);

    auto* os = parsed_obj.content().as_stream();
    if (os == nullptr)
        throw parse_exception{"Xref stream indirect object is not a stream"};

    (void)os->content();

    auto section = cross_reference_stream_parser::parse(*os);

    trailer trailer_obj{os->dictionary()};

    return {std::move(section), std::move(trailer_obj)};
}

} // namespace ripper::pdf::core
