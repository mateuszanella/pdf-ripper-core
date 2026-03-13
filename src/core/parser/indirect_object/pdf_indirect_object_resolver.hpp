#pragma once

#include <cstdint>
#include <expected>
#include <functional>
#include <utility>

#include "core/parser/parser.hpp"
#include "core/parser/indirect_object/indirect_object_resolution_context.hpp"

namespace ripper::core
{
    class pdf_indirect_object_resolver
    {
    public:
        explicit pdf_indirect_object_resolver(indirect_object_resolution_context context)
            : context_{context}
        {
        }

        const indirect_object_resolution_context &context() const noexcept
        {
            return context_;
        }

        std::expected<std::uint64_t, parser_error> resolve_offset(const indirect_reference &reference) const
        {
            const auto entry = context_.xref_table.find(reference);

            if (!entry.has_value())
            {
                return std::unexpected(parser_error::object_not_found);
            }

            return entry->offset();
        }

        template <typename PdfObject, typename ReferenceResolver, typename ObjectBuilder>
        std::expected<PdfObject, parser_error> resolve(
            ReferenceResolver &&resolve_reference,
            ObjectBuilder &&build_object) const
        {
            auto ref_result = std::invoke(std::forward<ReferenceResolver>(resolve_reference), context_);

            if (!ref_result)
            {
                return std::unexpected(ref_result.error());
            }

            auto offset_result = resolve_offset(*ref_result);

            if (!offset_result)
            {
                return std::unexpected(offset_result.error());
            }

            return std::invoke(std::forward<ObjectBuilder>(build_object), *ref_result, *offset_result);
        }

    private:
        indirect_object_resolution_context context_;
    };
}
