#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/pdf_object.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    class pages : public pdf_object
    {
    public:
        pages(const document &doc, indirect_reference ref) noexcept
            : pdf_object{doc, ref}
        {
        }

        pages(const document &doc, indirect_reference ref, std::optional<std::uint32_t> count) noexcept
            : pdf_object{doc, ref}, count_{count}
        {
        }

        std::expected<std::uint32_t, parser_error> count() const;

    private:
        std::optional<std::uint32_t> count_;
    };
}
