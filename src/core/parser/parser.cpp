#include "core/parser/parser.hpp"

#include "core/document.hpp"
#include "core/parser/cross_reference_table/cross_reference_table_parser.hpp"
#include "core/parser/cross_reference_table/default_cross_reference_table_parser.hpp"
#include "core/parser/document_structure/default_document_structure_parser.hpp"
#include "core/parser/document_structure/document_structure_parser.hpp"
#include "core/parser/header/header_parser.hpp"
#include "core/parser/indirect_object_resolver.hpp"
#include "core/parser/object_parser.hpp"
#include "core/parser/parser_manager.hpp"
#include "core/parser/trailer/default_trailer_parser.hpp"
#include "core/parser/trailer/trailer_parser.hpp"

#include <memory>
#include <utility>

namespace ripper::pdf::core
{
parser::parser(document& doc)
    : document_{doc}, manager_{std::make_unique<class parser_manager>(doc)}
{
}

parser::~parser() = default;

parser_manager& parser::manager()
{
    if (!manager_)
        manager_ = std::make_unique<class parser_manager>(document_);

    return *manager_;
}

header parser::header()
{
    return manager().header_parser().parse();
}

document_structure parser::structure()
{
    return manager().document_structure_parser().parse();
}

indirect_object parser::parse_object(indirect_reference ref, bool preload_stream)
{
    auto content = manager().object_resolver().resolve(ref);
    return manager().object_parser().parse(document_, ref, content, preload_stream);
}
} // namespace ripper::pdf::core
