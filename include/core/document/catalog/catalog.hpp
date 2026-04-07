#pragma once

#include <optional>

#include "core/document/pdf_object.hpp"
#include "core/document/catalog/pages/pages.hpp"

namespace ripper::core
{
    /**
     * @brief Represents the PDF document catalog (the /Root object).
     * Lazily resolves child objects via the owning document.
     */
    class catalog : public pdf_object
    {
    public:
        explicit catalog(const document &doc, indirect_reference ref, std::optional<indirect_reference> pages_ref) noexcept
            : pdf_object{doc, ref}, pages_ref_{pages_ref}
        {
        }

        explicit catalog(pdf_object obj, std::optional<indirect_reference> pages_ref) noexcept
            : pdf_object{std::move(obj)}, pages_ref_{pages_ref}
        {
        }

        std::expected<class pages, parser_error> pages();

    private:
        std::optional<indirect_reference> pages_ref_;
        std::optional<class pages> pages_;
    };
}
