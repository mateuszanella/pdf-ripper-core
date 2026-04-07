#include "core/document/catalog/catalog.hpp"

#include "core/document.hpp"
#include "core/document/catalog/pages/pages.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    std::expected<class pages, parser_error> catalog::pages()
    {
        if (pages_.has_value())
            return pages_.value();

        if (!pages_ref_.has_value())
            return std::unexpected(parser_error::missing_catalog);

        auto parsed = owner().parser().pages(*pages_ref_);
        if (!parsed)
            return std::unexpected(parsed.error());

        pages_.emplace(std::move(parsed.value()));

        return pages_.value();
    }
}
