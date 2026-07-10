#include "ripper/pdf/core/parser/parser.hpp"

#include "ripper/pdf/core/document.hpp"
#include "ripper/pdf/core/parser/document_structure/document_structure_parser.hpp"
#include "ripper/pdf/core/parser/header/header_parser.hpp"
#include "ripper/pdf/core/parser/indirect_object_resolver.hpp"
#include "ripper/pdf/core/parser/object_parser.hpp"
#include "ripper/pdf/core/parser/parser_manager.hpp"
#include "ripper/pdf/core/parser/trailer/default_trailer_parser.hpp"
#include "ripper/pdf/core/parser/trailer/trailer_parser.hpp"

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
    (void)preload_stream;
    return manager().object_parser().parse(document_, ref, content);
}
} // namespace ripper::pdf::core
