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
        catalog(const document &doc, indirect_reference ref, std::uint64_t offset,
                std::optional<class pages> pages) noexcept
            : pdf_object{doc, ref, offset}, pages_{std::move(pages)}
        {
        }

        /**
         * @brief Returns the Pages tree root, if present.
         */
        [[nodiscard]] const std::optional<class pages> &pages() const noexcept
        {
            return pages_;
        }

    private:
        std::optional<class pages> pages_;
    };
}
