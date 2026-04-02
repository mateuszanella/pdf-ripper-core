#pragma once

#include <optional>

#include "core/document/pdf_object.hpp"
#include "core/document/catalog/entities/pages.hpp"

namespace ripper::core
{
    /**
     * @brief Represents the PDF document catalog (the /Root object).
     * Lazily resolves child objects via the owning document.
     */
    class catalog : public pdf_object
    {
    public:
        explicit catalog(const document &doc, indirect_reference ref) noexcept
            : pdf_object{doc, ref}
        {
        }

        explicit catalog(pdf_object obj) noexcept
            : pdf_object{std::move(obj)}
        {
        }

        // /**
        //  * @brief Returns the Pages tree root, if present.
        //  */
        // [[nodiscard]] const std::optional<class pages> &pages() const noexcept
        // {
        //     return pages_;
        // }

    private:
        // std::optional<class pages> pages_;
    };
}
