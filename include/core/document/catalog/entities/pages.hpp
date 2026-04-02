#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "core/document/pdf_object.hpp"
#include "core/errors/parser/parser_error.hpp"

namespace ripper::core
{
    /**
     * @brief Represents the PDF Pages tree root object.
     * Lazily resolves its properties via the document.
     */
    class pages : public pdf_object
    {
    public:
        pages(const document &doc, indirect_reference ref) noexcept
            : pdf_object{doc, ref}
        {
        }

        /**
         * @brief Returns the total page count. Parsed lazily on first access.
         */
        [[nodiscard]] std::expected<std::uint32_t, parser_error> count() const;

    private:
        mutable std::optional<std::uint32_t> count_;
    };
}
